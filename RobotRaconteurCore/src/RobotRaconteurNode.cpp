// Copyright 2011-2019 Wason Technology, LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "RobotRaconteur/RobotRaconteurNode.h"
#include <boost/algorithm/string.hpp>
#include "RobotRaconteur/Client.h"
#include "RobotRaconteur/RobotRaconteurServiceIndex.h"
#include "RobotRaconteur/DataTypes.h"
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/filesystem.hpp>

#include "Discovery_private.h"
#include "RobotRaconteurNode_connector_private.h"

namespace RobotRaconteur
{
	static void RobotRaconteurNode_empty_handler() {}


	static void RobotRaconteurNode_empty_handler(RR_SHARED_PTR<RobotRaconteurException> ) {}


RobotRaconteurNode RobotRaconteurNode::m_s;
RR_SHARED_PTR<RobotRaconteurNode> RobotRaconteurNode::m_sp;
RR_WEAK_PTR<RobotRaconteurNode> RobotRaconteurNode::m_weak_sp;

bool RobotRaconteurNode::is_init=false;

RobotRaconteurNode::RobotRaconteurNode()
{
	is_shutdown=false;
	NodeID_set=false;
	NodeName_set=false;
	PeriodicCleanupTask_timerstarted=false;

	transport_count=0;
	

	EndpointInactivityTimeout = 600000;
	TransportInactivityTimeout = 600000;
	RequestTimeout = 15000;
	MemoryMaxTransferSize = 102400;
	instance_is_init=false;

	log_level = RobotRaconteur_LogLevel_Warning;

	//ROBOTRACONTEUR_LOG_TRACE_COMPONENT(node, Node, -1, "RobotRaconteurNode created");
	
}

void RobotRaconteurNode::Init()
{
	if (instance_is_init) return;

	{
		random_generator = RR_MAKE_SHARED<boost::random::random_device>();
	}

	instance_is_init=true;

	//Deal with possible race in boost::filesystem::path
	boost::filesystem::path::codecvt();

	m_Discovery = RR_MAKE_SHARED<detail::Discovery>(shared_from_this());

	ROBOTRACONTEUR_LOG_INFO_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode version " << ROBOTRACONTEUR_VERSION_TEXT << " initialized");
	
}

static void RobotRaconteurNode_emptydeleter(RobotRaconteurNode* n) {}

RobotRaconteurNode* RobotRaconteurNode::s()
{
	if(!is_init)
	{
		is_init=true;

		m_sp.reset(&m_s,&RobotRaconteurNode_emptydeleter);
		m_s._internal_accept_owner(&m_sp,&m_s);
		m_weak_sp = m_sp;
		m_s.Init();
	}
	return &m_s;
}

RR_SHARED_PTR<RobotRaconteurNode> RobotRaconteurNode::sp()
{
	RobotRaconteurNode::s();
	return m_sp;
}

RR_WEAK_PTR<RobotRaconteurNode> RobotRaconteurNode::weak_sp()
{
	return m_weak_sp;
}

NodeID RobotRaconteurNode::NodeID()
{
	if (!NodeID_set)
	{
		m_NodeID=RobotRaconteur::NodeID::NewUniqueID();
		NodeID_set=true;
		::RobotRaconteur::NodeID n = m_NodeID;
		
		ROBOTRACONTEUR_LOG_INFO_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode NodeID configured with random UUID " << n.ToString());
		return n;
	}

	return m_NodeID;
}

std::string RobotRaconteurNode::NodeName()
{
	if (!NodeName_set)
	{
		m_NodeName="";
		NodeName_set=true;
	}
	return m_NodeName;
}

bool RobotRaconteurNode::TryGetNodeID(RobotRaconteur::NodeID& id)
{
	
	if (!NodeID_set)
	{
		return false;
	}

	id = m_NodeID;
	return true;
}

bool RobotRaconteurNode::TryGetNodeName(std::string& node_name)
{
	
	if (!NodeName_set)
	{
		return false;
	}

	node_name = m_NodeName;
	return true;
}

void RobotRaconteurNode::SetNodeID(const RobotRaconteur::NodeID& id)
{
	
	if (NodeID_set)
	{
		
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode attempt to set NodeID when already set");
	 	throw InvalidOperationException("NodeID already set");
	}
	m_NodeID=id;
	NodeID_set=true;
	
	ROBOTRACONTEUR_LOG_INFO_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode NodeID set to UUID " << m_NodeID.ToString());
}

void RobotRaconteurNode::SetNodeName(boost::string_ref name)
{
	if (name.size() > 1024)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "NodeName too long");
	 	throw InvalidArgumentException("NodeName too long");
	}
	if(!boost::regex_match(name.begin(),name.end(),boost::regex("^[a-zA-Z][a-zA-Z0-9_\\.\\-]*$")))
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Invalid NodeName \"" << name << "\"");
		throw InvalidArgumentException("\"" + name + "\" is an invalid NodeName");
	}

	
	if (NodeName_set)
	{
		
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode attempt to set NodeName when already set");
		throw InvalidOperationException("NodeName already set");
	}
	m_NodeName = RR_MOVE(name.to_string());
	NodeName_set=true;

	
	ROBOTRACONTEUR_LOG_INFO_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode NodeName set to \"" << m_NodeName << "\"");
}

RR_SHARED_PTR<ServiceFactory> RobotRaconteurNode::GetServiceType(boost::string_ref servicename)
{
	
	RR_UNORDERED_MAP<std::string, RR_SHARED_PTR<ServiceFactory> >::iterator e1 = service_factories.find(servicename.to_string());
	if(e1==service_factories.end())
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Unknown service type \"" << servicename << "\"");
		throw ServiceException("Unknown service type");
	}
	return e1->second;

}

bool RobotRaconteurNode::IsServiceTypeRegistered(boost::string_ref servicename)
{
	RR_UNORDERED_MAP<std::string, RR_SHARED_PTR<ServiceFactory> >::iterator e1 = service_factories.find(servicename.to_string());
	return e1 != service_factories.end();
}

void RobotRaconteurNode::RegisterServiceType(RR_SHARED_PTR<ServiceFactory> factory)
{
	
	if (boost::ends_with(factory->GetServiceName(),"_signed")) throw ServiceException("Could not verify signed service definition");

	if(service_factories.count(factory->GetServiceName())!=0)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Service type already registered \"" << factory->GetServiceName() << "\"");
		throw ServiceException("Service type already registered");		
	}

	
	factory->ServiceDef()->CheckVersion();

	factory->SetNode(shared_from_this());

	service_factories.insert(std::make_pair(factory->GetServiceName(),factory));

	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "Service type registered \"" << factory->GetServiceName() << "\"");
}

void RobotRaconteurNode::UnregisterServiceType(boost::string_ref type)
{
	RR_UNORDERED_MAP<std::string, RR_SHARED_PTR<ServiceFactory> >::iterator e1 = service_factories.find(type.to_string());
	if (e1==service_factories.end())
	{ 
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Cannot unregister nonexistant service type \"" << type << "\"");
		throw InvalidArgumentException("Service type not registered");
	}
	service_factories.erase(e1);
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "Service type unregistered \"" << type << "\"");
}


std::vector<std::string> RobotRaconteurNode::GetRegisteredServiceTypes()
{
	std::vector<std::string> o;
	boost::copy(service_factories | boost::adaptors::map_keys, std::back_inserter(o));
	return o;
}

uint32_t RobotRaconteurNode::RegisterTransport(RR_SHARED_PTR<Transport> transport)
{
	{
		if (transport_count >= std::numeric_limits<uint32_t>::max())
			transport_count=0;
		else
			transport_count++;
		transport->TransportID=transport_count;
		transports.insert(std::make_pair(transport_count,transport));
	}

	RR_SHARED_PTR<ITransportTimeProvider> t=RR_DYNAMIC_POINTER_CAST<ITransportTimeProvider>(transport);
	if (t)
	{
		RR_SHARED_PTR<ITransportTimeProvider> t2=time_provider.lock();
		if (!t2)
		{
			time_provider=t;
		}

	}

	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "Node " << transport->GetUrlSchemeString() << " registered");

	return transport->TransportID;
}


RR_INTRUSIVE_PTR<MessageElementNestedElementList> RobotRaconteurNode::PackStructure(RR_INTRUSIVE_PTR<RRStructure> structure)
{
	return detail::packing::PackStructure(structure, this);
}

RR_INTRUSIVE_PTR<RRStructure> RobotRaconteurNode::UnpackStructure(RR_INTRUSIVE_PTR<MessageElementNestedElementList> structure)
{
	return detail::packing::UnpackStructure(structure, this);
}

RR_INTRUSIVE_PTR<MessageElementNestedElementList> RobotRaconteurNode::PackPodArray(RR_INTRUSIVE_PTR<RRPodBaseArray> a)
{
	return detail::packing::PackPodArray(a, this);
}

RR_INTRUSIVE_PTR<RRPodBaseArray> RobotRaconteurNode::UnpackPodArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> a)
{
	return detail::packing::UnpackPodArray(a, this);
}

RR_INTRUSIVE_PTR<MessageElementNestedElementList> RobotRaconteurNode::PackPodMultiDimArray(RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray> a)
{
	return detail::packing::PackPodMultiDimArray(a, this);
}

RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray> RobotRaconteurNode::UnpackPodMultiDimArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> a)
{
	return detail::packing::UnpackPodMultiDimArray(a, this);
}


RR_INTRUSIVE_PTR<MessageElementNestedElementList> RobotRaconteurNode::PackNamedArray(RR_INTRUSIVE_PTR<RRNamedBaseArray> a)
{
	return detail::packing::PackNamedArray(a, this);
}

RR_INTRUSIVE_PTR<RRNamedBaseArray> RobotRaconteurNode::UnpackNamedArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> a)
{
	return detail::packing::UnpackNamedArray(a, this);
}

RR_INTRUSIVE_PTR<MessageElementNestedElementList> RobotRaconteurNode::PackNamedMultiDimArray(RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray> a)
{
	return detail::packing::PackNamedMultiDimArray(a, this);
}

RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray> RobotRaconteurNode::UnpackNamedMultiDimArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> a)
{
	return detail::packing::UnpackNamedMultiDimArray(a, this);
}

RR_INTRUSIVE_PTR<MessageElementData> RobotRaconteurNode::PackVarType(RR_INTRUSIVE_PTR<RRValue> vardata)
{
	return detail::packing::PackVarType(vardata, this);
}

RR_INTRUSIVE_PTR<RRValue> RobotRaconteurNode::UnpackVarType(RR_INTRUSIVE_PTR<MessageElement> mvardata1)
{
	return detail::packing::UnpackVarType(mvardata1, this);
}

void RobotRaconteurNode::Shutdown()
{
	/*ROBOTRACONTEUR_ASSERT_MULTITHREADED(shared_from_this());

	ROBOTRACONTEUR_LOG_INFO_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode begin shutdown");

	{		
		if (!instance_is_init) return;
		if (is_shutdown) return;
		{
			is_shutdown = true;
		}

		std::vector<RR_SHARED_PTR<Endpoint> > endpointsv;
		{
			boost::copy(endpoints | boost::adaptors::map_values, std::back_inserter(endpointsv));
		}
	
		BOOST_FOREACH (RR_SHARED_PTR<Endpoint>& e, endpointsv)
		{
			try
			{
				RR_SHARED_PTR<ClientContext> e1=RR_DYNAMIC_POINTER_CAST<ClientContext>(e);
				if (e1)
					e1->AsyncClose(boost::bind(&RobotRaconteurNode_empty_handler));
			}
			catch (std::exception&) {}
		}
	

		{
			endpoints.clear();
		}

			//std::cout << "start transport close" << std::endl;

		{
			if (m_Discovery)
			{
				m_Discovery->Shutdown();
			}

		}

		{
			BOOST_FOREACH(RR_SHARED_PTR<Transport>& e, transports | boost::adaptors::map_values)
			{
				try
				{
					e->Close();
				}
				catch (std::exception&) {}
			}
	
			transports.clear();

		}

		{

			cleanupobjs.clear();
		}
	}

	{
		WallTimer::node_shutdown(this);
	}

	shutdown_listeners();

	{
		if (this->PeriodicCleanupTask_timer)
		{
			try
			{
			this->PeriodicCleanupTask_timer->Stop();
			}
			catch (std::exception&) {}
			this->PeriodicCleanupTask_timer->Clear();
			this->PeriodicCleanupTask_timer.reset();
		}
	}
			
	
	{
		exception_handler.clear();
	}

	discovery_updated_listeners.disconnect_all_slots();
	discovery_lost_listeners.disconnect_all_slots();
	
	ROBOTRACONTEUR_LOG_INFO_COMPONENT(weak_sp(), Node, -1, "RobotRaconteurNode shutdown complete");*/
}

RobotRaconteurNode::~RobotRaconteurNode()
{
	//Shutdown();
	
	
}

static std::string RobotRaconteurNode_log_msg_servicepath(RR_INTRUSIVE_PTR<Message>& m)
{
	if (m->entries.empty())
	{
		return "";
	}
	return m->entries[0]->ServicePath.str().to_string();
}

static std::string RobotRaconteurNode_log_msg_member(RR_INTRUSIVE_PTR<Message>& m)
{
	if (m->entries.empty())
	{
		return "";
	}
	return m->entries[0]->MemberName.str().to_string();
}

static uint16_t RobotRaconteurNode_log_msg_entrytype(RR_INTRUSIVE_PTR<Message>& m)
{
	if (m->entries.empty())
	{
		return 0;
	}
	return (uint16_t)m->entries[0]->EntryType;
}

static uint16_t RobotRaconteurNode_log_msg_error(RR_INTRUSIVE_PTR<Message>& m)
{
	if (m->entries.empty())
	{
		return 0;
	}
	return (uint16_t)m->entries[0]->Error;
}


#define ROBOTRACONTEUR_LOG_MESSAGE(log_cmd, node, source, msg_txt, m) \
	log_cmd(node,source,e->GetLocalEndpoint(),RobotRaconteurNode_log_msg_servicepath(m), \
		RobotRaconteurNode_log_msg_member(m), msg_txt << " from " << m->header->SenderNodeID.ToString() << " ep "  \
		<< m->header->SenderEndpoint << " to " << m->header->ReceiverNodeID.ToString() << " ep " << m->header->ReceiverEndpoint \
		<< " EntryType " << RobotRaconteurNode_log_msg_entrytype(m) << " Error " << RobotRaconteurNode_log_msg_error(m))


void RobotRaconteurNode::AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& callback)
{
	if (m->header->SenderNodeID != NodeID())
	{

		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Attempt to send message with invalid SenderNodeID");
		throw ConnectionException("Could not route message");
		
	}

	RR_SHARED_PTR<Endpoint> e;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(m->header->SenderEndpoint);
		if (e1==endpoints.end())
		{
			if (is_shutdown)
			{
				ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Attempt to send message after node shutdown");
				throw InvalidOperationException("Attempt to send message after node shutdown");
			}

			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Attempt to send message using invalid endpoint " << m->header->SenderEndpoint);
		 	throw InvalidEndpointException("Could not find endpoint");
		}
		e = e1->second;
	}

	RR_SHARED_PTR<Transport> c;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Transport> >::iterator e1 = transports.find(e->GetTransport());
		if (e1==transports.end())
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Transport, e->GetLocalEndpoint(), "Could not find transport to send message from endpoint " << e->GetLocalEndpoint());
		 	throw ConnectionException("Could not find transport");
		}
		c = e1->second;
	}

	c->AsyncSendMessage(m,callback);

	ROBOTRACONTEUR_LOG_MESSAGE(ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH,weak_sp(),Node,"Sending message",m)
}

static void empty_end_send_message(RR_SHARED_PTR<RobotRaconteurException>) {}

void RobotRaconteurNode::MessageReceived(RR_INTRUSIVE_PTR<Message> m)
{
	try
		{
		if (m->header->ReceiverNodeID != NodeID())
		{
				ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Received message with invalid ReceiverNodeID: " << m->header->ReceiverNodeID.ToString());
				RR_INTRUSIVE_PTR<Message> eret = GenerateErrorReturnMessage(m, MessageErrorType_NodeNotFound, "RobotRaconteur.NodeNotFound", "Could not find route to remote node");
				if (eret->entries.size() > 0)
				{
					boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)> cb = boost::bind(&empty_end_send_message,_1);
					AsyncSendMessage(eret, cb);
				}
		}

		else
		{
			
			RR_SHARED_PTR<Endpoint> e;
				
			{
				RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(m->header->ReceiverEndpoint);
				if (e1 != endpoints.end())
				{
					e = e1->second;
				}
			}

			if (e)
			{
				ROBOTRACONTEUR_LOG_MESSAGE(ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH,weak_sp(),Node,"Received message",m)
				e->MessageReceived(m);
			}
			else
			{			
				ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Received message with invalid ReceiverEndpoint: " << m->header->ReceiverEndpoint);
			RR_INTRUSIVE_PTR<Message> eret = GenerateErrorReturnMessage(m, MessageErrorType_InvalidEndpoint, "RobotRaconteur.InvalidEndpoint", "Invalid destination endpoint");
			if (eret->entries.size() > 0)
			{
				boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)> cb = boost::bind(&empty_end_send_message,_1);
				AsyncSendMessage(eret,cb);
			}
			}
		}
	}
	catch (std::exception& e)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Exception receiving message: " << e.what());
		HandleException(&e);
	}
}

void RobotRaconteurNode::TransportConnectionClosed(uint32_t endpoint)
{
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, endpoint, "Node notified that transport connection was closed");

	RR_SHARED_PTR<Endpoint> e;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(endpoint);
		if (e1 != endpoints.end())
		{
			e = e1->second;
		}
		else
		{
			return;
		}
	}

	e->TransportConnectionClosed(endpoint);

}

uint32_t RobotRaconteurNode::GetRequestTimeout()
{
	return RequestTimeout;
}
void RobotRaconteurNode::SetRequestTimeout(uint32_t timeout)
{
	RequestTimeout=timeout;
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "RequestTimeout set to: " << timeout << " ms");
}

uint32_t RobotRaconteurNode::GetTransportInactivityTimeout()
{
	return TransportInactivityTimeout;
}
void RobotRaconteurNode::SetTransportInactivityTimeout(uint32_t timeout)
{
	TransportInactivityTimeout=timeout;
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "TransportInactivityTimeout set to: " << timeout << " ms");
}

uint32_t RobotRaconteurNode::GetEndpointInactivityTimeout()
{
	return EndpointInactivityTimeout;
}

void RobotRaconteurNode::SetEndpointInactivityTimeout(uint32_t timeout)
{
	EndpointInactivityTimeout=timeout;
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "EndpointInactivityTimeout set to: " << timeout << " ms");
}

uint32_t RobotRaconteurNode::GetMemoryMaxTransferSize()
{
	return MemoryMaxTransferSize;
}

void RobotRaconteurNode::SetMemoryMaxTransferSize(uint32_t size)
{
	MemoryMaxTransferSize=size;
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "MemoryMaxTransferSize set to: " << size << " bytes");
}


const RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> RobotRaconteurNode::GetDynamicServiceFactory() 
{
	return dynamic_factory;
}

void RobotRaconteurNode::SetDynamicServiceFactory(RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> f)
{

	if (this->dynamic_factory != 0)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Dynamic service factory already set");
		throw InvalidOperationException("Dynamic service factory already set");
	}
	this->dynamic_factory = f;
}

RR_INTRUSIVE_PTR<Message> RobotRaconteurNode::GenerateErrorReturnMessage(RR_INTRUSIVE_PTR<Message> m, MessageErrorType err, boost::string_ref errname, boost::string_ref errdesc)
{
	RR_INTRUSIVE_PTR<Message> ret = CreateMessage();
	ret->header = CreateMessageHeader();
	ret->header->ReceiverNodeName = m->header->SenderNodeName;
	ret->header->SenderNodeName = m->header->ReceiverNodeName;
	ret->header->ReceiverNodeID = m->header->SenderNodeID;
	ret->header->ReceiverEndpoint = m->header->SenderEndpoint;
	ret->header->SenderEndpoint = m->header->ReceiverEndpoint;
	ret->header->SenderNodeID = m->header->ReceiverNodeID;
	BOOST_FOREACH (RR_INTRUSIVE_PTR<MessageEntry>& me, m->entries)
	{
		if ((static_cast<int32_t>(me->EntryType)) % 2 == 1)
		{
			RR_INTRUSIVE_PTR<MessageEntry> eret = CreateMessageEntry(static_cast<MessageEntryType>(me->EntryType+1), me->MemberName);
			eret->RequestID = me->RequestID;
			eret->ServicePath = me->ServicePath;
			eret->AddElement("errorname", stringToRRArray(errname));
			eret->AddElement("errorstring", stringToRRArray(errdesc));
			eret->Error = err;
			ret->entries.push_back(eret);
		}
	}
	return ret;
}

void RobotRaconteurNode::AsyncConnectService(boost::string_ref url, boost::string_ref username, RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > credentials, boost::function<void (RR_SHARED_PTR<ClientContext>,ClientServiceListenerEventType,RR_SHARED_PTR<void>)> listener, boost::string_ref objecttype, boost::function<void(RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	std::vector<std::string> urls;
	urls.push_back(RR_MOVE(url.to_string()));
	AsyncConnectService(urls,username,credentials,listener,objecttype,handler,timeout);
}

void RobotRaconteurNode::AsyncConnectService(const std::vector<std::string> &url, boost::string_ref username, RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > credentials, boost::function<void (RR_SHARED_PTR<ClientContext>,ClientServiceListenerEventType,RR_SHARED_PTR<void>)> listener, boost::string_ref objecttype, boost::function<void(RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	
	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "Begin ConnectService with candidate urls: " << boost::join(url, ", "));

	if (url.empty())
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "No urls specified for ConnectService");
		throw InvalidArgumentException("URL vector must not be empty for AsyncConnectService");
	}

	std::vector<RR_SHARED_PTR<Transport> > atransports;

	std::map<std::string,RR_WEAK_PTR<Transport> > connectors;
	{
		boost::copy(transports | boost::adaptors::map_values, std::back_inserter(atransports));		
	}

	
		BOOST_FOREACH(const std::string& e, url)
		{
			BOOST_FOREACH (RR_SHARED_PTR<Transport> end, atransports)
			{
				if (end == 0)
					continue;
				if (end->IsClient())
				{
					if (end->CanConnectService(e))
					{
						connectors.insert(std::make_pair(e,RR_WEAK_PTR<Transport>(end)));
				
					}
				}
			}
		}

		if (connectors.empty())
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "ConnectService could not find any valid transports for urls: " << boost::join(url, ", "));
		 	throw ConnectionException("Could not find any valid transports for requested connection URLs");
		}

		RR_SHARED_PTR<detail::RobotRaconteurNode_connector> connector=RR_MAKE_SHARED<detail::RobotRaconteurNode_connector>(shared_from_this());
		Post(boost::bind(&detail::RobotRaconteurNode_connector::connect, connector, connectors, username.to_string(), credentials, listener, objecttype, boost::protect(handler), timeout));
		return;	
}

void RobotRaconteurNode::AsyncDisconnectService(RR_SHARED_PTR<RRObject> obj, boost::function<void()> handler)
{
	if (!obj) return;
	RR_SHARED_PTR<ServiceStub> stub = rr_cast<ServiceStub>(obj);
	
	RR_SHARED_PTR<ClientContext> c = stub->GetContext();

	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, c->GetLocalEndpoint(), "Client Disconnecting");
	c->AsyncClose(RR_MOVE(handler));
}

std::map<std::string, RR_INTRUSIVE_PTR<RRValue> > RobotRaconteurNode::GetServiceAttributes(RR_SHARED_PTR<RRObject> obj)
{
	RR_SHARED_PTR<ServiceStub> stub = rr_cast<ServiceStub>(obj);
	return stub->GetContext()->GetAttributes();
}

uint32_t RobotRaconteurNode::RegisterEndpoint(RR_SHARED_PTR<Endpoint> e)
{
	
	{
		boost::random::uniform_int_distribution<uint32_t> distribution(0,std::numeric_limits<uint32_t>::max());
		uint32_t id;
		{
		do
		{
			id=distribution(*random_generator);
		}
		while (endpoints.count(id)!=0 || recent_endpoints.count(id)!=0);
		}
		e->SetLocalEndpoint(id);
		endpoints.insert(std::make_pair(id, e));

		ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, e->GetLocalEndpoint(), "Endpoint registered, RemoteNodeID " << e->GetRemoteNodeID().ToString() << " ep " << e->GetRemoteEndpoint());

		return id;
		

	}
}

void RobotRaconteurNode::DeleteEndpoint(RR_SHARED_PTR<Endpoint> e)
{

	try
	{
		
		{			
			RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(e->GetLocalEndpoint());
			if (e1 != endpoints.end())			
			{
				endpoints.erase(e1);
				recent_endpoints.insert(std::make_pair(e->GetLocalEndpoint(),NowUTC()));
			}
		}
	}
	catch (std::exception& exp)
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, e->GetLocalEndpoint(), "Error deleting endpoint: " << exp.what());
	}

	try
	{
		RR_SHARED_PTR<Transport> c;		
		{
			RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Transport> >::iterator e1 = transports.find(e->GetTransport());
			if (e1 != transports.end())
			{
				c = e1->second;
			}			
		}
		if (c) c->CloseTransportConnection(e);
	}
	catch (std::exception& exp)
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, e->GetLocalEndpoint(), "Error closing transport connection for deleted endpoint: " << exp.what());
	}

	ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, e->GetLocalEndpoint(), "Endpoint deleted");
}

void RobotRaconteurNode::CheckConnection(uint32_t endpoint)
{	
	RR_SHARED_PTR<Endpoint> e;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(endpoint);
		if (e1 == endpoints.end())
		{
			if (is_shutdown)
			{				
				throw InvalidOperationException("Node has been shut down");
			}
			
		 	throw InvalidEndpointException("Invalid Endpoint");
		}
		e = e1->second;
	}

	RR_SHARED_PTR<Transport> c;		
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Transport> >::iterator e1 = transports.find(e->GetTransport());
		if (e1 == transports.end()) throw ConnectionException("Transport connection not found");
		c = e1->second;
	}
	c->CheckConnection(endpoint);
}

std::vector<NodeDiscoveryInfo> RobotRaconteurNode::GetDetectedNodes() 
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	return m_Discovery->GetDetectedNodes();
}

void RobotRaconteurNode::NodeDetected(const NodeDiscoveryInfo& info)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	m_Discovery->NodeDetected(info);
}

void RobotRaconteurNode::AsyncUpdateDetectedNodes(const std::vector<std::string>& schemes, boost::function<void()> handler, int32_t timeout)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->AsyncUpdateDetectedNodes(schemes, handler, timeout);
}


void RobotRaconteurNode::NodeAnnouncePacketReceived(boost::string_ref packet)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	m_Discovery->NodeAnnouncePacketReceived(packet);
}

void RobotRaconteurNode::CleanDiscoveredNodes()
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	m_Discovery->CleanDiscoveredNodes();
}

uint32_t RobotRaconteurNode::GetNodeDiscoveryMaxCacheCount()
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	return m_Discovery->GetNodeDiscoveryMaxCacheCount();
}
void RobotRaconteurNode::SetNodeDiscoveryMaxCacheCount(uint32_t count)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	m_Discovery->SetNodeDiscoveryMaxCacheCount(count);
}

RR_SHARED_PTR<ServiceSubscription> RobotRaconteurNode::SubscribeService(const std::vector<std::string>& service_types, RR_SHARED_PTR<ServiceSubscriptionFilter> filter)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	return m_Discovery->SubscribeService(service_types, filter);
}

RR_SHARED_PTR<ServiceInfo2Subscription> RobotRaconteurNode::SubscribeServiceInfo2(const std::vector<std::string>& service_types, RR_SHARED_PTR<ServiceSubscriptionFilter> filter)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
		throw InvalidOperationException("Node not init");
	}
	return m_Discovery->SubscribeServiceInfo2(service_types, filter);
}

void RobotRaconteurNode::FireNodeDetected(RR_SHARED_PTR<NodeDiscoveryInfo> node, RR_SHARED_PTR<std::vector<ServiceInfo2> > services)
{
	discovery_updated_listeners(*node, *services);
}

void RobotRaconteurNode::FireNodeLost(RR_SHARED_PTR<NodeDiscoveryInfo> node)
{
	discovery_lost_listeners(*node);
}

std::string RobotRaconteurNode::SelectRemoteNodeURL(const std::vector<std::string>& urls)
{
	BOOST_FOREACH (const std::string& e, urls)
	{
		if (boost::starts_with(e,"rr+local://")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(e, "rr+pci://")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(e,"rr+usb://")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e), "rrs+tcp://127.0.0.1")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e), "rrs+tcp://[::1]")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e), "rrs+tcp://localhost")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e), "rrs+tcp://[fe80")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e), "rrs+tcp://")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e),"rr+tcp://127.0.0.1")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e),"rr+tcp://[::1]")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e),"rr+tcp://localhost")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e),"rr+tcp://[fe80")) return e;
	}

	BOOST_FOREACH(const std::string& e, urls)
	{
		if (boost::starts_with(boost::to_lower_copy(e),"rr+tcp://")) return e;
	}

	return urls.at(0);
}


void RobotRaconteurNode::AsyncFindServiceByType(boost::string_ref servicetype, const std::vector<std::string>& transportschemes, boost::function<void(RR_SHARED_PTR<std::vector<ServiceInfo2> >) > handler, int32_t timeout)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
	 	throw InvalidOperationException("Node not init");
	}
	m_Discovery->AsyncFindServiceByType(servicetype, transportschemes, handler, timeout);

}

void RobotRaconteurNode::AsyncFindNodeByID(const RobotRaconteur::NodeID& id, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<NodeInfo2> >) > handler, int32_t timeout)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
		throw InvalidOperationException("Node not init");
	}
	m_Discovery->AsyncFindNodeByID(id, transportschemes, handler, timeout);
}

void RobotRaconteurNode::AsyncFindNodeByName(boost::string_ref name, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<NodeInfo2> >) > handler, int32_t timeout)
{
	if (!m_Discovery)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Node not init");
		throw InvalidOperationException("Node not init");
	}
	m_Discovery->AsyncFindNodeByName(name, transportschemes, handler, timeout);
}

void RobotRaconteurNode::AsyncRequestObjectLock(RR_SHARED_PTR<RRObject> obj, RobotRaconteurObjectLockFlags flags, boost::function<void(RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{		
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Can only lock object opened through Robot Raconteur");
	 	throw InvalidArgumentException("Can only lock object opened through Robot Raconteur");
	}
	s->GetContext()->AsyncRequestObjectLock(obj,flags,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncReleaseObjectLock(RR_SHARED_PTR<RRObject> obj, boost::function<void(RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{		
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Can only lock object opened through Robot Raconteur");
		throw InvalidArgumentException("Can only unlock object opened through Robot Raconteur");
	}
	s->GetContext()->AsyncReleaseObjectLock(obj,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::PeriodicCleanupTask(const TimerEvent& err)
{
	
	if (err.stopped) return;

	{
		boost::posix_time::ptime now=NowUTC();
		
		std::vector<RR_SHARED_PTR<Endpoint> > e;
		
		{
			boost::copy(endpoints | boost::adaptors::map_values, std::back_inserter(e));
						
			for(std::map<uint32_t,boost::posix_time::ptime>::iterator e=recent_endpoints.begin(); e!=recent_endpoints.end(); )
			{
				int32_t seconds=boost::numeric_cast<int32_t>((now-e->second).total_seconds());
				if (seconds > 300)
				{
					recent_endpoints.erase(e++);
				}
				else
				{
					e++;
				}
			}

		}

		BOOST_FOREACH (RR_SHARED_PTR<Endpoint>& ee, e)
		{
			try
			{
				ee->PeriodicCleanupTask();
			}
			catch (std::exception&)
			{
			}
		}
		

		std::vector<RR_SHARED_PTR<Transport> > c;		
		{
			boost::copy(transports | boost::adaptors::map_values, std::back_inserter(c));
		}
		
		BOOST_FOREACH (RR_SHARED_PTR<Transport>& cc, c)
		{
			try
			{
				cc->PeriodicCleanupTask();
			}
			catch (std::exception&)
			{
			}
		}
			
		try
		{

			CleanDiscoveredNodes();
		}
		catch (std::exception&)
		{
		}

		std::vector<RR_SHARED_PTR<IPeriodicCleanupTask> > cleanobjs;
		{
			
			boost::copy(cleanupobjs, std::back_inserter(cleanobjs));						
		}
		BOOST_FOREACH (RR_SHARED_PTR<IPeriodicCleanupTask>& t, cleanobjs)
		{
			try
			{
				t->PeriodicCleanupTask();
			}
			catch (std::exception&)
			{
			}
		}
	}
	
}

void RobotRaconteurNode::AddPeriodicCleanupTask(RR_SHARED_PTR<IPeriodicCleanupTask> task)
{
	
	cleanupobjs.push_back(task);
}

void RobotRaconteurNode::RemovePeriodicCleanupTask(RR_SHARED_PTR<IPeriodicCleanupTask> task)
{
	cleanupobjs.remove(task);
}


void RobotRaconteurNode::AsyncFindObjRefTyped(RR_SHARED_PTR<RRObject> obj, boost::string_ref objref, boost::string_ref objecttype, boost::function<void (RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can be have objrefs");
		throw InvalidArgumentException("Only service stubs can be have objrefs");
	}
	s->AsyncFindObjRefTyped(objref,objecttype,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncFindObjRefTyped(RR_SHARED_PTR<RRObject> obj, boost::string_ref objref, boost::string_ref index, boost::string_ref objecttype, boost::function<void (RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can be have objrefs");
		throw InvalidArgumentException("Only service stubs can be have objrefs");
	}
	s->AsyncFindObjRefTyped(objref,index,objecttype,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncFindObjectType(RR_SHARED_PTR<RRObject> obj, boost::string_ref n, boost::function<void (RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can be have objrefs");
		throw InvalidArgumentException("Only service stubs can be have objrefs");
	}
	s->AsyncFindObjectType(n,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncFindObjectType(RR_SHARED_PTR<RRObject> obj, boost::string_ref n, boost::string_ref i, boost::function<void (RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can be have objrefs");
		throw InvalidArgumentException("Only service stubs can be have objrefs");
	}
	s->AsyncFindObjectType(n,i,RR_MOVE(handler),timeout);
}

std::vector<std::string> RobotRaconteurNode::GetPulledServiceTypes(RR_SHARED_PTR<RRObject> obj)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can be have objrefs");
		throw InvalidArgumentException("Only service stubs can be have objrefs");
	}
	return s->GetContext()->GetPulledServiceTypes();
}

RR_SHARED_PTR<ServiceFactory> RobotRaconteurNode::GetPulledServiceType(RR_SHARED_PTR<RRObject> obj, boost::string_ref type)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s)
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can be have objrefs");
		throw InvalidArgumentException("Only service stubs can be have objrefs");
	}
	return s->GetContext()->GetPulledServiceType(type);
}


void RobotRaconteurNode::StartPeriodicCleanupTask(RR_SHARED_PTR<RobotRaconteurNode> node)
{
	
	node->PeriodicCleanupTask_timer=node->CreateTimer(boost::posix_time::seconds(5),boost::bind(&RobotRaconteurNode::PeriodicCleanupTask,node,_1));
	node->PeriodicCleanupTask_timer->Start();
}

void RobotRaconteurNode::SetExceptionHandler(boost::function<void (const std::exception*)> handler)
{
	exception_handler=handler;
}

boost::function<void (const std::exception*)> RobotRaconteurNode::GetExceptionHandler()
{
	return exception_handler;
}

void RobotRaconteurNode::HandleException(const std::exception* exp)
{
	if (exp==NULL) return;

	boost::function<void (const std::exception*)> h;
	{
		h=exception_handler;
	}

	if (h)
	{
		try
		{
			h(exp);
		}
		catch (...) {}
	}
	else
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(),Node,-1,"Uncaught exception in unknown handler: " << exp->what())
	}
}

bool RobotRaconteurNode::TryHandleException(RR_WEAK_PTR<RobotRaconteurNode> node, const std::exception* exp)
{
	RR_SHARED_PTR<RobotRaconteurNode> node1 = node.lock();
	if (!node1) return false;
	node1->HandleException(exp);
	return true;
}

boost::posix_time::ptime RobotRaconteurNode::NowUTC()
{

	RR_SHARED_PTR<ITransportTimeProvider> t=time_provider.lock();

	if (t)
	{
		return t->NowUTC();
	}
	else
	{
		return boost::posix_time::microsec_clock::universal_time();
	}
}



RR_SHARED_PTR<Timer> RobotRaconteurNode::CreateTimer(const boost::posix_time::time_duration& period, boost::function<void (const TimerEvent&)> handler, bool oneshot)
{
	RR_SHARED_PTR<ITransportTimeProvider> t=time_provider.lock();
	if (!t)
	{
		RR_SHARED_PTR<Timer> timer = RR_MAKE_SHARED<WallTimer>(period,handler,oneshot,shared_from_this());
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "Timer created using WallTimer");
		return timer;
	}
	else
	{
		RR_SHARED_PTR<Timer> timer = t->CreateTimer(period,handler,oneshot);
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT(weak_sp(), Node, -1, "Timer created using transport provider");
		return timer;
	}
}

void RobotRaconteurNode::DownCastAndThrowException(RobotRaconteurException& exp)
{
	if (exp.ErrorCode != MessageErrorType_RemoteError)
	{
		RobotRaconteurExceptionUtil::DownCastAndThrowException(exp);
	}

	std::string type=exp.Error;
	if (!boost::contains(type,"."))
	{
		throw exp;
	}
	boost::tuple<boost::string_ref,boost::string_ref> stype=SplitQualifiedName(type);
	if (!IsServiceTypeRegistered(stype.get<0>()))
	{
		throw exp;
	}

	GetServiceType(stype.get<0>())->DownCastAndThrowException(exp);

}

RR_SHARED_PTR<RobotRaconteurException> RobotRaconteurNode::DownCastException(RR_SHARED_PTR<RobotRaconteurException> exp)
{
	if (!exp) return exp;

	if (exp->ErrorCode != MessageErrorType_RemoteError)
	{		
		return RobotRaconteurExceptionUtil::DownCastException(exp);
	}

	std::string type=exp->Error;
	if (!boost::contains(type,"."))
	{
		return exp;
	}
	boost::tuple<boost::string_ref,boost::string_ref> stype=SplitQualifiedName(type);
	if (!IsServiceTypeRegistered(stype.get<0>()))
	{
		return exp;
	}

	return GetServiceType(stype.get<0>())->DownCastException(exp);
}

std::string RobotRaconteurNode::GetServicePath(RR_SHARED_PTR<RRObject> obj)
{
	if (!(dynamic_cast<ServiceStub*>(obj.get()) != 0))
	{
		ROBOTRACONTEUR_LOG_DEBUG_COMPONENT(weak_sp(), Node, -1, "Only service stubs can have objrefs");
		throw InvalidArgumentException("Only service stubs can have objrefs");
	}
	RR_SHARED_PTR<ServiceStub> s = rr_cast<ServiceStub>(obj);
	return s->ServicePath;
}

bool RobotRaconteurNode::IsEndpointLargeTransferAuthorized(uint32_t endpoint)
{
	try
	{
		RR_SHARED_PTR<Endpoint> e;
		{
			RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(endpoint);
			if (e1 == endpoints.end()) return false;
			e = e1->second;
		}

		RR_SHARED_PTR<ClientContext> c = RR_DYNAMIC_POINTER_CAST<ClientContext>(e);
		if (c)
		{
			return true;
		}

		return false;
		
	}
	catch (std::exception&)
	{		
		return false;
	}

}

std::string RobotRaconteurNode::GetRobotRaconteurVersion()
{
	return ROBOTRACONTEUR_VERSION_TEXT;

}

std::string RobotRaconteurNode::GetRandomString(size_t count)
{
	std::string o;
	std::string strvals = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	boost::random::uniform_int_distribution<uint32_t> distribution(0, boost::numeric_cast<uint32_t>(strvals.size() - 1));
	for (size_t i = 0; i<count; i++)
	{
		o += strvals.at(distribution(*random_generator));
	}
	return o;
}

// Emscripten Functions
void RobotRaconteurNode::Post(boost::function<void()> f)
{
	RR_SHARED_PTR<Timer> t=CreateTimer(boost::posix_time::milliseconds(0), boost::bind(f), true);
	t->Start();
}

void RobotRaconteurNode::AsyncSleep(const boost::posix_time::time_duration& d, boost::function<void()> handler)
{
	RR_SHARED_PTR<Timer> t=CreateTimer(d, boost::bind(handler), true);
	t->Start();
}

bool RobotRaconteurNode::CompareLogLevel(RobotRaconteur_LogLevel record_level)
{
	return record_level >= log_level;
}

void RobotRaconteurNode::LogMessage(RobotRaconteur_LogLevel level, const std::string& message)
{
	RRLogRecord r;
	r.Node=shared_from_this();
	r.Level=level;
	r.Component=RobotRaconteur_LogComponent_Default;
	r.Endpoint=0;
	r.Message=message;
	

	LogRecord(r);
}

void RobotRaconteurNode::LogRecord(const RRLogRecord& record)
{
	
	
	if (record.Level < log_level)
	{
		return;
	}
	
	
	
	
	

	if (log_handler)
	{
		log_handler->HandleLogRecord(record);
		return;
	}
	
	
	std::cerr << record << std::endl; 

}

RobotRaconteur_LogLevel RobotRaconteurNode::GetLogLevel()
{
	
	return log_level;
}
void RobotRaconteurNode::SetLogLevel(RobotRaconteur_LogLevel level)
{
	
	log_level = level;
}

RobotRaconteur_LogLevel RobotRaconteurNode::SetLogLevelFromEnvVariable(const std::string& env_variable_name)
{
	
	char* loglevel_c = std::getenv(env_variable_name.c_str());
	if (!loglevel_c) return RobotRaconteur_LogLevel_Warning;
	std::string loglevel(loglevel_c);
	if (loglevel== "DISABLE")
	{
		log_level = RobotRaconteur_LogLevel_Disable;
		return RobotRaconteur_LogLevel_Disable;
	}

	if (loglevel == "FATAL")
	{
		log_level = RobotRaconteur_LogLevel_Fatal;
		return RobotRaconteur_LogLevel_Fatal;
	}

	if (loglevel == "ERROR")
	{
		log_level = RobotRaconteur_LogLevel_Error;
		return RobotRaconteur_LogLevel_Error;
	}

	if (loglevel == "WARNING")
	{
		log_level = RobotRaconteur_LogLevel_Warning;
		return RobotRaconteur_LogLevel_Warning;
	}

	if (loglevel == "INFO")
	{
		log_level = RobotRaconteur_LogLevel_Info;
		return RobotRaconteur_LogLevel_Info;
	}

	if (loglevel == "DEBUG")
	{
		log_level = RobotRaconteur_LogLevel_Debug;
		return RobotRaconteur_LogLevel_Debug;
	}

	if (loglevel == "TRACE")
	{
		log_level = RobotRaconteur_LogLevel_Trace;
		return RobotRaconteur_LogLevel_Trace;
	}

	

	ROBOTRACONTEUR_LOG_WARNING_COMPONENT(weak_sp(), Node, -1, "Invalid log level specified in environmental variable: " << loglevel);

	return log_level;
}

RR_SHARED_PTR<LogRecordHandler> RobotRaconteurNode::GetLogRecordHandler()
{
	
	return log_handler;
}
void RobotRaconteurNode::SetLogRecordHandler(RR_SHARED_PTR<LogRecordHandler> handler)
{
	
	log_handler = handler;
}

}

