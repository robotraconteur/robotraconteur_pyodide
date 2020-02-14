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

#include "RobotRaconteur/Endpoint.h"
#include "RobotRaconteur/RobotRaconteurNode.h"
#include "RobotRaconteur/DataTypes.h"
#include "RobotRaconteur/ErrorUtil.h"

namespace RobotRaconteur
{


uint32_t Endpoint::GetLocalEndpoint()
{	
	return m_LocalEndpoint;	
}

void Endpoint::SetLocalEndpoint(uint32_t endpoint)
{	
	m_LocalEndpoint=(endpoint);
}

uint32_t Endpoint::GetRemoteEndpoint()
{	
	return m_RemoteEndpoint;	
}

void Endpoint::SetRemoteEndpoint(uint32_t endpoint)
{	
	m_RemoteEndpoint=(endpoint);
}

std::string Endpoint::GetRemoteNodeName()
{
	std::string ret=m_RemoteNodeName;
	return ret;
}

void Endpoint::SetRemoteNodeName(boost::string_ref name)
{
	m_RemoteNodeName = RR_MOVE(name.to_string());
}

NodeID Endpoint::GetRemoteNodeID()
{	
	NodeID ret = m_RemoteNodeID;
	return ret;
}

void Endpoint::SetRemoteNodeID(NodeID id)
{	
	m_RemoteNodeID=id;
}

uint32_t Endpoint::GetTransport()
{	
	return m_transport;	
}

void Endpoint::SetTransport(uint32_t transport)
{	
	m_transport=(transport);
}

RR_SHARED_PTR<ITransportConnection> Endpoint::GetTransportConnection()
{
	return m_TransportConnection.lock();
}
void Endpoint::SetTransportConnection(RR_SHARED_PTR<ITransportConnection> c)
{
	m_TransportConnection = c;
}

boost::posix_time::ptime Endpoint::GetLastMessageReceivedTime()
{	
	return m_LastMessageReceivedTime;
	
}

void Endpoint::SetLastMessageReceivedTime(boost::posix_time::ptime time)
{	
	m_LastMessageReceivedTime=(time);
}

boost::posix_time::ptime Endpoint::GetLastMessageSentTime()
{	
	return m_LastMessageSentTime;	
}

void Endpoint::SetLastMessageSentTime(boost::posix_time::ptime time)
{	
	m_LastMessageSentTime=(time);
}

void Endpoint::AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& callback)
{
	if (!m->header)
		m->header = CreateMessageHeader();

	if (m->entries.size()==1 && m->entries.at(0)->EntryType <=500)
	{
		m->header->ReceiverNodeName = GetRemoteNodeName();
		m->header->SenderNodeName = GetNode()->NodeName();
	}
	m->header->SenderEndpoint = GetLocalEndpoint();
	m->header->ReceiverEndpoint = GetRemoteEndpoint();

	m->header->SenderNodeID = GetNode()->NodeID();
	m->header->ReceiverNodeID = GetRemoteNodeID();


		
	{		
		m->header->MessageID = MessageNumber;

		MessageNumber = static_cast<uint16_t>((MessageNumber == (static_cast<uint16_t>(std::numeric_limits<uint16_t>::max()))) ? 0 : MessageNumber + 1);
	}

	SetLastMessageSentTime(boost::posix_time::microsec_clock::universal_time());

	GetNode()->AsyncSendMessage(m,callback);
}


	void Endpoint::PeriodicCleanupTask()
	{

	}

	uint32_t Endpoint::EndpointCapability(boost::string_ref name)
	{
		return static_cast<uint32_t>(0);
	}

	Endpoint::Endpoint(RR_SHARED_PTR<RobotRaconteurNode> node)
	{
		m_LocalEndpoint=(0);
		m_RemoteEndpoint=(0);
		m_RemoteNodeName = "";
		m_RemoteNodeID=NodeID::GetAny();
		//TransportConnection = 0;
		m_LastMessageReceivedTime=(boost::posix_time::microsec_clock::universal_time());
		m_LastMessageSentTime=(boost::posix_time::microsec_clock::universal_time());
		MessageNumber=(0);
		//MessageNumberLock = RR_MAKE_SHARED<Object>();
		this->node=node;
		m_transport=(std::numeric_limits<uint32_t>::max());
	}

	RR_SHARED_PTR<RobotRaconteurNode> Endpoint::GetNode()
	{
		RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
		if (!n) throw InvalidOperationException("Node has been released");
		return n;
	}

	void Endpoint::TransportConnectionClosed(uint32_t endpoint)
	{

	}

}
