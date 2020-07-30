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
#include <boost/shared_array.hpp>

#pragma once

namespace RobotRaconteur
{

    class BrowserWebSocketTransportConnection;

    class BrowserWebSocketTransport : public Transport, public RR_ENABLE_SHARED_FROM_THIS<BrowserWebSocketTransport>
    {
        public:

            friend class BrowserWebSocketTransportConnection;

		    RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<ITransportConnection> > TransportConnections;
		
            BrowserWebSocketTransport(RR_SHARED_PTR<RobotRaconteurNode> node=RobotRaconteurNode::sp());

            virtual ~BrowserWebSocketTransport();

            virtual bool IsServer() const;
            virtual bool IsClient() const;
            
            virtual int32_t GetDefaultReceiveTimeout();
            virtual void SetDefaultReceiveTimeout(int32_t milliseconds);
            virtual int32_t GetDefaultConnectTimeout();
            virtual void SetDefaultConnectTimeout(int32_t milliseconds);

            virtual std::string GetUrlSchemeString() const;

            virtual void AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& callback);

            virtual void AsyncCreateTransportConnection(boost::string_ref url, RR_SHARED_PTR<Endpoint> e, boost::function<void (RR_SHARED_PTR<ITransportConnection>, RR_SHARED_PTR<RobotRaconteurException> ) >& callback);

            virtual void CloseTransportConnection(RR_SHARED_PTR<Endpoint> e);

            virtual bool CanConnectService(boost::string_ref url);
		
            virtual void Close();

		    virtual void CheckConnection(uint32_t endpoint);

            virtual void PeriodicCleanupTask();

            uint32_t TransportCapability(boost::string_ref name);

            virtual void MessageReceived(RR_INTRUSIVE_PTR<Message> m);

            virtual int32_t GetDefaultHeartbeatPeriod();
            virtual void SetDefaultHeartbeatPeriod(int32_t milliseconds);

            virtual bool GetDisableMessage4();
            virtual void SetDisableMessage4(bool d);

            virtual bool GetDisableStringTable();
            virtual void SetDisableStringTable(bool d);

            virtual bool GetDisableAsyncMessageIO();
            virtual void SetDisableAsyncMessageIO(bool d);

        protected:

            virtual void register_transport(RR_SHARED_PTR<ITransportConnection> connection);
		    virtual void erase_transport(RR_SHARED_PTR<ITransportConnection> connection);

            bool closed;
            int32_t heartbeat_period;
            int32_t default_connect_timeout;
            int32_t default_receive_timeout;            
            bool disable_message4;
            bool disable_string_table;
            bool disable_async_message_io;
    };    
}