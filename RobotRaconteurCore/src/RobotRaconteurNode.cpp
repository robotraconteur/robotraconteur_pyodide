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
	
}

static void RobotRaconteurNode_emptydeleter(RobotRaconteurNode* n) {}

RobotRaconteurNode* RobotRaconteurNode::s()
{
	if(!is_init)
	{
		is_init=true;

		m_sp.reset(&m_s,&RobotRaconteurNode_emptydeleter);
		m_s._internal_accept_owner(&m_sp,&m_s);
		m_s.Init();
	}
	return &m_s;
}

RR_SHARED_PTR<RobotRaconteurNode> RobotRaconteurNode::sp()
{
	RobotRaconteurNode::s();
	return m_sp;
}

NodeID RobotRaconteurNode::NodeID()
{
	if (!NodeID_set)
	{
		m_NodeID=RobotRaconteur::NodeID::NewUniqueID();
		NodeID_set=true;
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

void RobotRaconteurNode::SetNodeID(const RobotRaconteur::NodeID& id)
{
	if (NodeID_set) throw InvalidOperationException("NodeID already set");
	m_NodeID=id;
	NodeID_set=true;
}

void RobotRaconteurNode::SetNodeName(const std::string& name)
{
	if (name.size() > 1024) throw InvalidArgumentException("NodeName too long");
	if(!boost::regex_match(name,boost::regex("^[a-zA-Z][a-zA-Z0-9_\\.\\-]*$")))
	{
		throw InvalidArgumentException("\"" + name + "\" is an invalid NodeName");
	}
	if (NodeName_set) throw InvalidOperationException("NodeName already set");
	m_NodeName=name;
	NodeName_set=true;
}

RR_SHARED_PTR<ServiceFactory> RobotRaconteurNode::GetServiceType(const std::string& servicename)
{
	RR_UNORDERED_MAP<std::string, RR_SHARED_PTR<ServiceFactory> >::iterator e1 = service_factories.find(servicename);
	if(e1==service_factories.end())
	{
		throw ServiceException("Unknown service type");
	}
	return e1->second;

}

bool RobotRaconteurNode::IsServiceTypeRegistered(const std::string& servicename)
{
	RR_UNORDERED_MAP<std::string, RR_SHARED_PTR<ServiceFactory> >::iterator e1 = service_factories.find(servicename);
	return e1 != service_factories.end();
}

void RobotRaconteurNode::RegisterServiceType(RR_SHARED_PTR<ServiceFactory> factory)
{
	
	if (boost::ends_with(factory->GetServiceName(),"_signed")) throw ServiceException("Could not verify signed service definition");

	if(service_factories.count(factory->GetServiceName())!=0)
	{
		throw ServiceException("Service type already registered");

		
	}

	
	factory->ServiceDef()->CheckVersion();

	factory->SetNode(shared_from_this());



	service_factories.insert(std::make_pair(factory->GetServiceName(),factory));
}

void RobotRaconteurNode::UnregisterServiceType(const std::string& type)
{
	RR_UNORDERED_MAP<std::string, RR_SHARED_PTR<ServiceFactory> >::iterator e1 = service_factories.find(type);
	if (e1==service_factories.end()) throw InvalidArgumentException("Service type not registered");
	service_factories.erase(e1);
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
	return transport->TransportID;
}


RR_INTRUSIVE_PTR<MessageElementStructure> RobotRaconteurNode::PackStructure(RR_INTRUSIVE_PTR<RRStructure> structure)
{
	
	if (!structure) return RR_INTRUSIVE_PTR<MessageElementStructure>();

	std::string type=structure->RRType();

	std::string servicetype=SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory=GetServiceType(servicetype);

	return factory->PackStructure(structure);

}

RR_INTRUSIVE_PTR<RRStructure> RobotRaconteurNode::UnpackStructure(RR_INTRUSIVE_PTR<MessageElementStructure> structure)
{
	if (!structure) return RR_INTRUSIVE_PTR<RRStructure>();

	std::string type=structure->Type;

	
	std::string servicetype=SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory=GetServiceType(servicetype);

	return rr_cast<RRStructure>(factory->UnpackStructure(structure));
	
}

RR_INTRUSIVE_PTR<MessageElementPodArray> RobotRaconteurNode::PackPodArray(RR_INTRUSIVE_PTR<RRPodBaseArray> a)
{

	if (!a) return RR_INTRUSIVE_PTR<MessageElementPodArray>();

	std::string type = a->RRElementTypeString();

	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return factory->PackPodArray(a);
}

RR_INTRUSIVE_PTR<RRPodBaseArray> RobotRaconteurNode::UnpackPodArray(RR_INTRUSIVE_PTR<MessageElementPodArray> a)
{
	if (!a) return RR_INTRUSIVE_PTR<RRPodBaseArray>();

	std::string type = a->Type;


	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return rr_cast<RRPodBaseArray>(factory->UnpackPodArray(a));
}

RR_INTRUSIVE_PTR<MessageElementPodMultiDimArray> RobotRaconteurNode::PackPodMultiDimArray(RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray> a)
{

	if (!a) return RR_INTRUSIVE_PTR<MessageElementPodMultiDimArray>();

	std::string type = a->RRElementTypeString();

	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return factory->PackPodMultiDimArray(a);
}

RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray> RobotRaconteurNode::UnpackPodMultiDimArray(RR_INTRUSIVE_PTR<MessageElementPodMultiDimArray> a)
{
	if (!a) return RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray>();

	std::string type = a->Type;


	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return rr_cast<RRPodBaseMultiDimArray>(factory->UnpackPodMultiDimArray(a));
}


RR_INTRUSIVE_PTR<MessageElementNamedArray> RobotRaconteurNode::PackNamedArray(RR_INTRUSIVE_PTR<RRNamedBaseArray> a)
{

	if (!a) return RR_INTRUSIVE_PTR<MessageElementNamedArray>();

	std::string type = a->RRElementTypeString();

	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return factory->PackNamedArray(a);
}

RR_INTRUSIVE_PTR<RRNamedBaseArray> RobotRaconteurNode::UnpackNamedArray(RR_INTRUSIVE_PTR<MessageElementNamedArray> a)
{
	if (!a) return RR_INTRUSIVE_PTR<RRNamedBaseArray>();

	std::string type = a->Type;


	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return rr_cast<RRNamedBaseArray>(factory->UnpackNamedArray(a));
}

RR_INTRUSIVE_PTR<MessageElementNamedMultiDimArray> RobotRaconteurNode::PackNamedMultiDimArray(RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray> a)
{

	if (!a) return RR_INTRUSIVE_PTR<MessageElementNamedMultiDimArray>();

	std::string type = a->RRElementTypeString();

	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return factory->PackNamedMultiDimArray(a);
}

RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray> RobotRaconteurNode::UnpackNamedMultiDimArray(RR_INTRUSIVE_PTR<MessageElementNamedMultiDimArray> a)
{
	if (!a) return RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray>();

	std::string type = a->Type;


	std::string servicetype = SplitQualifiedName(type).get<0>();
	//std::string structuretype=res[1];

	RR_SHARED_PTR<ServiceFactory> factory = GetServiceType(servicetype);

	return rr_cast<RRNamedBaseMultiDimArray>(factory->UnpackNamedMultiDimArray(a));
}

RR_INTRUSIVE_PTR<MessageElementData> RobotRaconteurNode::PackVarType(RR_INTRUSIVE_PTR<RRValue> vardata)
{

	if (!vardata) return RR_INTRUSIVE_PTR<MessageElementData>();

	std::string type=vardata->RRType();

	std::string t1="RobotRaconteur.RRArray";
	if (type.compare(0,t1.length(),t1)==0)
	{
		return rr_cast<MessageElementData>(vardata);
	}

	std::string t2="RobotRaconteur.RRMap<int32_t>";
	if (type==t2)
	{
		return PackMapType<int32_t,RRValue>(vardata);
	}

	std::string t3="RobotRaconteur.RRMap<string>";
	if (type==t3)
	{
		return PackMapType<std::string,RRValue>(vardata);
	}
	
	std::string t6="RobotRaconteur.RRMap";
	if (type.compare(0,t6.size(),t6)==0)
	{
		//Unknown keytype type for map
		throw DataTypeException("Invalid map keytype");	
	}	

	std::string t5="RobotRaconteur.RRMultiDimArray";
	if (type.compare(0,t5.length(),t5)==0)
	{

		if (type=="RobotRaconteur.RRMultiDimArray<double>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<double> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<single>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<float> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<int8>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<int8_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<uint8>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<uint8_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<int16>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<int16_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<uint16>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<uint16_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<int32>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<int32_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<uint32>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<uint32_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<int64>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<int64_t> >(vardata));
		if (type=="RobotRaconteur.RRMultiDimArray<uint64>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<uint64_t> >(vardata));
		if (type == "RobotRaconteur.RRMultiDimArray<cdouble>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<cdouble> >(vardata));
		if (type == "RobotRaconteur.RRMultiDimArray<csingle>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<cfloat> >(vardata));
		if (type == "RobotRaconteur.RRMultiDimArray<bool>")
			return PackMultiDimArray(rr_cast<RRMultiDimArray<rr_bool> >(vardata));
		throw DataTypeException("Invalid MultiDimArray type");
	}

	std::string t8="RobotRaconteur.RRList";
	if (type==t8)
	{
		return PackListType<RRValue>(vardata);
	}

	std::string t9 = "RobotRaconteur.RRPodArray";
	if (type == t9)
	{
		return PackPodArray(rr_cast<RRPodBaseArray>(vardata));
	}

	std::string t11 = "RobotRaconteur.RRPodMultiDimArray";
	if (type == t11)
	{
		return PackPodMultiDimArray(rr_cast<RRPodBaseMultiDimArray>(vardata));
	}

	std::string t12 = "RobotRaconteur.RRNamedArray";
	if (type == t12)
	{
		return PackNamedArray(rr_cast<RRNamedBaseArray>(vardata));
	}

	std::string t13 = "RobotRaconteur.RRNamedMultiDimArray";
	if (type == t13)
	{
		return PackNamedMultiDimArray(rr_cast<RRNamedBaseMultiDimArray>(vardata));
	}

	return PackStructure(rr_cast<RRStructure>(vardata));
}

RR_INTRUSIVE_PTR<RRValue> RobotRaconteurNode::UnpackVarType(RR_INTRUSIVE_PTR<MessageElement> mvardata1)
{
	if (!mvardata1) return RR_INTRUSIVE_PTR<RRValue>();
	if (mvardata1->ElementType==DataTypes_void_t) return RR_INTRUSIVE_PTR<RRValue>();

	RR_INTRUSIVE_PTR<MessageElementData> mvardata=mvardata1->GetData();

	DataTypes type=mvardata->GetTypeID();

	if (IsTypeRRArray(type))
	{
		return rr_cast<RRValue>(mvardata);
	}

	if (type==DataTypes_structure_t)
	{
		return UnpackStructure(rr_cast<MessageElementStructure>(mvardata));
	}

	if (type==DataTypes_vector_t)
	{
		return UnpackMapType<int32_t,RRValue>(rr_cast<MessageElementMap<int32_t> >(mvardata));
	}

	if (type==DataTypes_dictionary_t)
	{
		return UnpackMapType<std::string,RRValue>(rr_cast<MessageElementMap<std::string> >(mvardata));
	}

	if (type==DataTypes_multidimarray_t)
	{
		DataTypes type1=MessageElement::FindElement(mvardata1->CastData<MessageElementMultiDimArray>()->Elements,"array")->ElementType;

		switch (type1)
		{	
		
		case DataTypes_double_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<double>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_single_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<float>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_int8_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<int8_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_uint8_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<uint8_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_int16_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<int16_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_uint16_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<uint16_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_int32_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<int32_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_uint32_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<uint32_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_int64_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<int64_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_uint64_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<uint64_t>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_cdouble_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<cdouble>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_csingle_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<cfloat>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		case DataTypes_bool_t:
			return rr_cast<RRValue>(UnpackMultiDimArray<rr_bool>(rr_cast<MessageElementMultiDimArray>(mvardata)));
		default:
			throw DataTypeException("Invalid data type");
			
		}


		throw DataTypeException("Invalid MultiDimArray type");
	}

	if (type==DataTypes_list_t)
	{
		return UnpackListType<RRValue>(rr_cast<MessageElementList >(mvardata));
	}
		
	if (type == DataTypes_pod_array_t)
	{
		return UnpackPodArray(rr_cast<MessageElementPodArray>(mvardata));
	}

	if (type == DataTypes_pod_multidimarray_t)
	{
		return UnpackPodMultiDimArray(rr_cast<MessageElementPodMultiDimArray>(mvardata));
	}

	if (type == DataTypes_namedarray_array_t)
	{
		return UnpackNamedArray(rr_cast<MessageElementNamedArray>(mvardata));
	}

	if (type == DataTypes_namedarray_multidimarray_t)
	{
		return UnpackNamedMultiDimArray(rr_cast<MessageElementNamedMultiDimArray>(mvardata));
	}

	throw DataTypeException("Unknown data type");
}

void RobotRaconteurNode::Shutdown()
{
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
	
}

RobotRaconteurNode::~RobotRaconteurNode()
{
	Shutdown();
	
	
}


void RobotRaconteurNode::SendMessage(RR_INTRUSIVE_PTR<Message> m)
{

	if (m->header->SenderNodeID != NodeID())
	{	
			throw ConnectionException("Could not route message");		
	}
		
	RR_SHARED_PTR<Endpoint> e;		
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(m->header->SenderEndpoint);
		if (e1==endpoints.end()) throw InvalidEndpointException("Could not find endpoint");
		e = e1->second;
	}

	RR_SHARED_PTR<Transport> c;			
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Transport> >::iterator e1 = transports.find(e->GetTransport());
		if (e1==transports.end()) throw ConnectionException("Could not find transport");
		c = e1->second;
	}	

	c->SendMessage(m);

}

void RobotRaconteurNode::AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& callback)
{
	if (m->header->SenderNodeID != NodeID())
	{

		
			throw ConnectionException("Could not route message");
		
	}

	RR_SHARED_PTR<Endpoint> e;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(m->header->SenderEndpoint);
		if (e1 == endpoints.end()) throw InvalidEndpointException("Could not find endpoint");
		e = e1->second;
	}

	RR_SHARED_PTR<Transport> c;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Transport> >::iterator e1 = transports.find(e->GetTransport());
		if (e1 == transports.end()) throw ConnectionException("Could not find transport");
		c = e1->second;
	}

	c->AsyncSendMessage(m,callback);

}

void RobotRaconteurNode::MessageReceived(RR_INTRUSIVE_PTR<Message> m)
{
	try
		{
		if (m->header->ReceiverNodeID != NodeID())
		{

				RR_INTRUSIVE_PTR<Message> eret = GenerateErrorReturnMessage(m, MessageErrorType_NodeNotFound, "RobotRaconteur.NodeNotFound", "Could not find route to remote node");
				if (eret->entries.size() > 0)
					SendMessage(eret);
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
				e->MessageReceived(m);
			}
			else
			{			
				
			RR_INTRUSIVE_PTR<Message> eret = GenerateErrorReturnMessage(m, MessageErrorType_InvalidEndpoint, "RobotRaconteur.InvalidEndpoint", "Invalid destination endpoint");
			if (eret->entries.size() > 0)
				SendMessage(eret);				
			}
		}
	}
	catch (std::exception& e)
	{
		HandleException(&e);
	}
}

void RobotRaconteurNode::TransportConnectionClosed(uint32_t endpoint)
{
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
}

uint32_t RobotRaconteurNode::GetTransportInactivityTimeout()
{
	return TransportInactivityTimeout;
}
void RobotRaconteurNode::SetTransportInactivityTimeout(uint32_t timeout)
{
	TransportInactivityTimeout=timeout;
}

uint32_t RobotRaconteurNode::GetEndpointInactivityTimeout()
{
	return EndpointInactivityTimeout;
}

void RobotRaconteurNode::SetEndpointInactivityTimeout(uint32_t timeout)
{
	EndpointInactivityTimeout=timeout;
}

uint32_t RobotRaconteurNode::GetMemoryMaxTransferSize()
{
	return MemoryMaxTransferSize;
}

void RobotRaconteurNode::SetMemoryMaxTransferSize(uint32_t size)
{
	MemoryMaxTransferSize=size;
}


const RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> RobotRaconteurNode::GetDynamicServiceFactory() 
{
	return dynamic_factory;
}

void RobotRaconteurNode::SetDynamicServiceFactory(RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> f)
{

	if (this->dynamic_factory != 0)
		throw InvalidOperationException("Dynamic service factory already set");
	this->dynamic_factory = f;
}

RR_INTRUSIVE_PTR<Message> RobotRaconteurNode::GenerateErrorReturnMessage(RR_INTRUSIVE_PTR<Message> m, MessageErrorType err, const std::string &errname, const std::string &errdesc)
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

void RobotRaconteurNode::AsyncConnectService(const std::string &url, const std::string &username, RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > credentials, boost::function<void (RR_SHARED_PTR<ClientContext>,ClientServiceListenerEventType,RR_SHARED_PTR<void>)> listener, const std::string& objecttype, boost::function<void(RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	std::vector<std::string> urls;
	urls.push_back(url);
	AsyncConnectService(urls,username,credentials,listener,objecttype,handler,timeout);
}

void RobotRaconteurNode::AsyncConnectService(const std::vector<std::string> &url, const std::string &username, RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > credentials, boost::function<void (RR_SHARED_PTR<ClientContext>,ClientServiceListenerEventType,RR_SHARED_PTR<void>)> listener, const std::string& objecttype, boost::function<void(RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	
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

		if (connectors.empty()) throw ConnectionException("Could not find any valid transports for requested connection URLs");

		RR_SHARED_PTR<detail::RobotRaconteurNode_connector> connector=RR_MAKE_SHARED<detail::RobotRaconteurNode_connector>(shared_from_this());
		Post(boost::bind(&detail::RobotRaconteurNode_connector::connect, connector, connectors, username, credentials, listener, objecttype, boost::protect(handler), timeout));
		return;	
}

void RobotRaconteurNode::AsyncDisconnectService(RR_SHARED_PTR<RRObject> obj, boost::function<void()> handler)
{
	if (!obj) return;
	RR_SHARED_PTR<ServiceStub> stub = rr_cast<ServiceStub>(obj);
	
	stub->GetContext()->AsyncClose(RR_MOVE(handler));
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
	catch (std::exception&)
	{
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
	catch (std::exception&)
	{
	}	
}

void RobotRaconteurNode::CheckConnection(uint32_t endpoint)
{	
	RR_SHARED_PTR<Endpoint> e;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> >::iterator e1 = endpoints.find(endpoint);
		if (e1 == endpoints.end()) throw InvalidEndpointException("Invalid Endpoint");
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
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	return m_Discovery->GetDetectedNodes();
}

void RobotRaconteurNode::NodeDetected(const NodeDiscoveryInfo& info)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->NodeDetected(info);
}

void RobotRaconteurNode::AsyncUpdateDetectedNodes(const std::vector<std::string>& schemes, boost::function<void()> handler, int32_t timeout)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->AsyncUpdateDetectedNodes(schemes, handler, timeout);
}


void RobotRaconteurNode::NodeAnnouncePacketReceived(const std::string& packet)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->NodeAnnouncePacketReceived(packet);
}

void RobotRaconteurNode::CleanDiscoveredNodes()
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->CleanDiscoveredNodes();
}

uint32_t RobotRaconteurNode::GetNodeDiscoveryMaxCacheCount()
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	return m_Discovery->GetNodeDiscoveryMaxCacheCount();
}
void RobotRaconteurNode::SetNodeDiscoveryMaxCacheCount(uint32_t count)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->SetNodeDiscoveryMaxCacheCount(count);
}

RR_SHARED_PTR<ServiceSubscription> RobotRaconteurNode::SubscribeService(const std::vector<std::string>& service_types, RR_SHARED_PTR<ServiceSubscriptionFilter> filter)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	return m_Discovery->SubscribeService(service_types, filter);
}

RR_SHARED_PTR<ServiceInfo2Subscription> RobotRaconteurNode::SubscribeServiceInfo2(const std::vector<std::string>& service_types, RR_SHARED_PTR<ServiceSubscriptionFilter> filter)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
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


void RobotRaconteurNode::AsyncFindServiceByType(const std::string &servicetype, const std::vector<std::string>& transportschemes, boost::function<void(RR_SHARED_PTR<std::vector<ServiceInfo2> >) > handler, int32_t timeout)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->AsyncFindServiceByType(servicetype, transportschemes, handler, timeout);

}

void RobotRaconteurNode::AsyncFindNodeByID(const RobotRaconteur::NodeID& id, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<NodeInfo2> >) > handler, int32_t timeout)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->AsyncFindNodeByID(id, transportschemes, handler, timeout);
}

void RobotRaconteurNode::AsyncFindNodeByName(const std::string& name, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<NodeInfo2> >) > handler, int32_t timeout)
{
	if (!m_Discovery) throw InvalidOperationException("Node not init");
	m_Discovery->AsyncFindNodeByName(name, transportschemes, handler, timeout);
}

void RobotRaconteurNode::AsyncRequestObjectLock(RR_SHARED_PTR<RRObject> obj, RobotRaconteurObjectLockFlags flags, boost::function<void(RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{		
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Can only lock object opened through Robot Raconteur");
	s->GetContext()->AsyncRequestObjectLock(obj,flags,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncReleaseObjectLock(RR_SHARED_PTR<RRObject> obj, boost::function<void(RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{		
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Can only unlock object opened through Robot Raconteur");
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

		BOOST_FOREACH (RR_SHARED_PTR<IPeriodicCleanupTask>& t, cleanupobjs)
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

void RobotRaconteurNode::AsyncFindObjRefTyped(RR_SHARED_PTR<RRObject> obj, const std::string& objref, const std::string& objecttype, boost::function<void (RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Only service stubs can be have objrefs");
	s->AsyncFindObjRefTyped(objref,objecttype,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncFindObjRefTyped(RR_SHARED_PTR<RRObject> obj, const std::string& objref, const std::string& index, const std::string& objecttype, boost::function<void (RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Only service stubs can be have objrefs");
	s->AsyncFindObjRefTyped(objref,index,objecttype,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncFindObjectType(RR_SHARED_PTR<RRObject> obj, const std::string &n, boost::function<void (RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Only service stubs can be have objrefs");
	s->AsyncFindObjectType(n,RR_MOVE(handler),timeout);
}

void RobotRaconteurNode::AsyncFindObjectType(RR_SHARED_PTR<RRObject> obj, const std::string &n, const std::string &i, boost::function<void (RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Only service stubs can be have objrefs");
	s->AsyncFindObjectType(n,i,RR_MOVE(handler),timeout);
}

std::vector<std::string> RobotRaconteurNode::GetPulledServiceTypes(RR_SHARED_PTR<RRObject> obj)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Only service stubs can be have objrefs");
	return s->GetContext()->GetPulledServiceTypes();
}

RR_SHARED_PTR<ServiceFactory> RobotRaconteurNode::GetPulledServiceType(RR_SHARED_PTR<RRObject> obj, const std::string& type)
{
	RR_SHARED_PTR<ServiceStub> s = RR_DYNAMIC_POINTER_CAST<ServiceStub>(obj);
	if (!s) throw InvalidArgumentException("Only service stubs can be have objrefs");
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
		return RR_MAKE_SHARED<WallTimer>(period,handler,oneshot,shared_from_this());
	}
	else
	{
		return t->CreateTimer(period,handler,oneshot);
	}
}

void RobotRaconteurNode::AsyncSleep(const boost::posix_time::time_duration& duration, boost::function<void()> handler)
{
	RobotRaconteurNode::SetTimeout(((double)duration.total_microseconds())*1e-3, boost::bind(handler));
}

void RobotRaconteurNode::DownCastAndThrowException(RobotRaconteurException& exp)
{
	std::string type=exp.Error;
	if (!boost::contains(type,"."))
	{
		return;
	}
	boost::tuple<std::string,std::string> stype=SplitQualifiedName(type);
	if (!IsServiceTypeRegistered(stype.get<0>()))
	{
		return;
	}

	GetServiceType(stype.get<0>())->DownCastAndThrowException(exp);

}

RR_SHARED_PTR<RobotRaconteurException> RobotRaconteurNode::DownCastException(RR_SHARED_PTR<RobotRaconteurException> exp)
{
	if (!exp) return exp;
	std::string type=exp->Error;
	if (!boost::contains(type,"."))
	{
		return exp;
	}
	boost::tuple<std::string,std::string> stype=SplitQualifiedName(type);
	if (!IsServiceTypeRegistered(stype.get<0>()))
	{
		return exp;
	}

	return GetServiceType(stype.get<0>())->DownCastException(exp);
}

std::string RobotRaconteurNode::GetServicePath(RR_SHARED_PTR<RRObject> obj)
{
	if (!(dynamic_cast<ServiceStub*>(obj.get()) != 0))
		throw InvalidArgumentException("Only service stubs can be have objrefs");
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
	//TODO: implement this
	throw NotImplementedException("Post not implemented");
}

long RobotRaconteurNode::SetTimeout(double msecs, boost::function<void(boost::system::error_code)> f)
{
	//TODO: implement this
	throw NotImplementedException("SetTimeout not implemented");
}
void RobotRaconteurNode::ClearTimeout(long timer)
{
	//TODO: implement this
	throw NotImplementedException("ClearTimeout not implemented");
}

}

