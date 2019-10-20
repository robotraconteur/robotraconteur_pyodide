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

#pragma once

#include "RobotRaconteur/NodeID.h"

#include "RobotRaconteur/Error.h"
#include "RobotRaconteur/DataTypes.h"
#include "RobotRaconteur/Message.h"
#include <boost/date_time.hpp>

//Workaround for Windows headers
#ifdef SendMessage
#undef SendMessage
#endif

namespace RobotRaconteur
{

	class ROBOTRACONTEUR_CORE_API RobotRaconteurNode;
	class ROBOTRACONTEUR_CORE_API ITransportConnection;

	class ROBOTRACONTEUR_CORE_API Endpoint : private boost::noncopyable
	{
	private:		
		uint32_t m_LocalEndpoint = 0;
		uint32_t m_RemoteEndpoint = 0;
		std::string m_RemoteNodeName;
		NodeID m_RemoteNodeID;
		uint32_t m_transport = 0;
		RR_WEAK_PTR<ITransportConnection> m_TransportConnection;

		boost::posix_time::ptime m_LastMessageReceivedTime;
		boost::posix_time::ptime m_LastMessageSentTime;

		uint16_t MessageNumber = 0;

	public:
		uint32_t GetLocalEndpoint();
		void SetLocalEndpoint(uint32_t endpoint);	
	
		uint32_t GetRemoteEndpoint();
		void SetRemoteEndpoint(uint32_t endpoint);

		std::string GetRemoteNodeName();
		void SetRemoteNodeName(std::string name);	

		NodeID GetRemoteNodeID();
		void SetRemoteNodeID(NodeID id);

		uint32_t GetTransport();
		void SetTransport(uint32_t transport);

		virtual RR_SHARED_PTR<ITransportConnection> GetTransportConnection();
		virtual void SetTransportConnection(RR_SHARED_PTR<ITransportConnection> c);	

		boost::posix_time::ptime GetLastMessageReceivedTime();
		void SetLastMessageReceivedTime(boost::posix_time::ptime time);
	
		boost::posix_time::ptime GetLastMessageSentTime();
		void SetLastMessageSentTime(boost::posix_time::ptime time);		
			
		virtual void AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& callback);
				
		virtual void MessageReceived(RR_INTRUSIVE_PTR<Message> m) = 0;

		virtual void PeriodicCleanupTask();
		
		virtual void TransportConnectionClosed(uint32_t endpoint);

	
	public:
		virtual uint32_t EndpointCapability(const std::string &name);
	
	public:
		Endpoint(RR_SHARED_PTR<RobotRaconteurNode> node);
		

		virtual ~Endpoint() {}

		RR_SHARED_PTR<RobotRaconteurNode> GetNode();

	protected:
		RR_WEAK_PTR<RobotRaconteurNode> node;

	};


#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES
	using EndpointPtr = RR_SHARED_PTR<Endpoint>;
	using EndpointConstPtr = RR_SHARED_PTR<const Endpoint>;
#endif
	

}