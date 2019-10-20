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
#include <list>
#include <boost/bind/protect.hpp>
#include <boost/shared_array.hpp>
#include "RobotRaconteur/AsyncMessageIO.h"

#pragma once

namespace RobotRaconteur
{

	namespace detail
	{
		class StringTable;




		class ASIOStreamBaseTransport : public ITransportConnection, public RR_ENABLE_SHARED_FROM_THIS<ASIOStreamBaseTransport>
		{

		protected:


			struct message_queue_entry
			{
				RR_INTRUSIVE_PTR<Message> message;
				boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)> callback;
			};

			bool connected = false;

			boost::shared_array<uint8_t> sendbuf;
			size_t sendbuf_len;
			bool sending;
			bool send_pause_request;
			bool send_paused;
			boost::function<void(const boost::system::error_code&)> send_pause_request_handler;

			std::list<message_queue_entry > send_queue;
			size_t send_message_size;
			
			boost::posix_time::ptime tlastsend;
			boost::posix_time::ptime tlastrecv;

			uint8_t streamseed[8];
			uint32_t recv_message_size;
			boost::shared_array<uint8_t> recvbuf;
			size_t recvbuf_len;
			size_t recvbuf_pos;
			size_t recvbuf_end;
			bool recv_pause_request;
			bool recv_paused;
			bool receiving;
			bool send_version3;
			boost::function<void(const boost::system::error_code&)> recv_pause_request_handler;

			RR_SHARED_PTR<Timer> heartbeat_timer;

			uint32_t ReceiveTimeout;
			uint32_t HeartbeatPeriod;

			bool SendHeartbeat;

			bool CheckStreamCapability_closed;
			bool CheckStreamCapability_waiting;
			boost::function<void(uint32_t, RR_SHARED_PTR<RobotRaconteurException>)> CheckStreamCapability_callback;
			std::queue<boost::tuple<std::string, boost::function<void(uint32_t, RR_SHARED_PTR<RobotRaconteurException>)> > > CheckStreamCapability_queue;

			RR_SHARED_PTR<Timer> CheckStreamCapability_timer;

			bool streamop_closed;
			bool streamop_waiting;
			boost::function<void(RR_SHARED_PTR<RRObject>, RR_SHARED_PTR<RobotRaconteurException>)> streamop_callback;
			std::queue<boost::tuple<std::string, RR_SHARED_PTR<RRObject>, boost::function<void(RR_SHARED_PTR<RRObject>, RR_SHARED_PTR<RobotRaconteurException>)> > > streamop_queue;

			RR_SHARED_PTR<Timer> streamop_timer;
			NodeID RemoteNodeID;

			NodeID target_nodeid;
			std::string target_nodename;

			int32_t max_message_size;

			bool send_large_transfer_authorized;
			bool recv_large_transfer_authorized;
						
			bool server;

			bool disable_message3;
			bool disable_async_io;

			mutable_buffers active_recv_bufs;

			RR_SHARED_PTR<AsyncMessageReader> async_reader;

			size_t async_recv_size;
			size_t async_recv_pos;
			uint16_t async_recv_version;
			size_t async_recv_continue_buf_count;

			RR_SHARED_PTR<AsyncMessageWriter> async_writer;

			uint16_t async_send_version;
			const_buffers async_send_bufs;

			uint32_t active_capabilities_message2_basic;
			uint32_t active_capabilities_message3_basic;			

		protected:

			ASIOStreamBaseTransport(RR_SHARED_PTR<RobotRaconteurNode> node);

			RR_WEAK_PTR<RobotRaconteurNode> node;
		public:

			virtual RR_SHARED_PTR<RobotRaconteurNode> GetNode();

		private:
			ASIOStreamBaseTransport(const ASIOStreamBaseTransport& that);

		protected:

			class AsyncAttachStream_args : public RRObject
			{
			public:
				NodeID nodeid;
				std::string nodename;

				AsyncAttachStream_args(const NodeID& nodeid_, const std::string& nodename_)
				{
					nodeid = nodeid_;
					nodename = nodename_;
				}

				virtual std::string RRType()
				{
					return "RobotRaconteur::ASIOStreamBaseTransport::AsyncAttachStream_args";
				}

			};

			virtual void AsyncAttachStream(bool server, const NodeID& target_nodeid, const std::string& target_nodename, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);
		public:
			virtual void AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);
			


		protected:

			void SimpleAsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);

			void SimpleAsyncEndSendMessage(RR_SHARED_PTR<RobotRaconteurException> err);

			virtual void AsyncAttachStream1(RR_SHARED_PTR<RRObject> parameter, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);


			virtual void BeginSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);
			virtual void BeginSendMessage1(RR_INTRUSIVE_PTR<Message> m, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);


			virtual void EndSendMessage(size_t startpos, const boost::system::error_code& error, size_t bytes_transferred, RR_INTRUSIVE_PTR<Message> m, size_t m_len, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback, boost::shared_array<uint8_t> buf);
			virtual void EndSendMessage1();

			virtual void EndSendMessage2(const boost::system::error_code& error, size_t bytes_transferred, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& callback);

			virtual void AsyncPauseSend(boost::function<void(const boost::system::error_code&)>& handler);
			virtual void AsyncResumeSend();


			//virtual void EndReceiveMessage(const boost::system::error_code& error,
			  //size_t bytes_transferred,  boost::shared_array<uint8_t> buf);

			//virtual void EndReceiveMessage2(RR_SHARED_PTR<RobotRaconteur::Message> message);

			virtual void BeginReceiveMessage1();

			virtual void EndReceiveMessage1(size_t startpos, const boost::system::error_code& error,
				size_t bytes_transferred);


			virtual void EndReceiveMessage2(size_t startpos, const boost::system::error_code& error,
				size_t bytes_transferred, size_t message_len, boost::shared_array<uint8_t> buf);

			virtual void EndReceiveMessage3(RR_INTRUSIVE_PTR<Message> message);
			virtual void EndReceiveMessage4();

			virtual void EndReceiveMessage5(const boost::system::error_code& error, size_t bytes_transferred);

			virtual void AsyncPauseReceive(boost::function<void(const boost::system::error_code&)>& handler);
			virtual void AsyncResumeReceive();

			virtual void Close();

			virtual void heartbeat_timer_func(const TimerEvent& e);

		public:
			virtual void MessageReceived(RR_INTRUSIVE_PTR<Message> m) = 0;

			virtual bool IsConnected();

			virtual uint32_t StreamCapabilities(const std::string &name);

			virtual void AsyncCheckStreamCapability(const std::string &name, boost::function<void(uint32_t, RR_SHARED_PTR<RobotRaconteurException>)>& callback);

		protected:

			virtual void BeginCheckStreamCapability(const std::string &name, boost::function<void(uint32_t, RR_SHARED_PTR<RobotRaconteurException>)>& callback);

			void CheckStreamCapability_EndSendMessage(RR_SHARED_PTR<RobotRaconteurException> err);

			static void CheckStreamCapability_timercallback(RR_WEAK_PTR<ASIOStreamBaseTransport> t, const TimerEvent& e);

			void CheckStreamCapability_MessageReceived(RR_INTRUSIVE_PTR<Message> m);

		public:

			void AsyncStreamOp(const std::string &command, RR_SHARED_PTR<RRObject> args, boost::function<void(RR_SHARED_PTR<RRObject>, RR_SHARED_PTR<RobotRaconteurException>)>&);

			virtual void PeriodicCleanupTask();

			virtual NodeID GetRemoteNodeID();

		protected:

			virtual void BeginStreamOp(const std::string &command, RR_SHARED_PTR<RRObject> args, boost::function<void(RR_SHARED_PTR<RRObject>, RR_SHARED_PTR<RobotRaconteurException>)>&);

			virtual RR_INTRUSIVE_PTR<MessageEntry> PackStreamOpRequest(const std::string &command, RR_SHARED_PTR<RRObject> args);

			virtual void StreamOp_EndSendMessage(RR_SHARED_PTR<RobotRaconteurException> err);

			static void StreamOp_timercallback(RR_WEAK_PTR<ASIOStreamBaseTransport> t, const TimerEvent& e);

			virtual void StreamOpMessageReceived(RR_INTRUSIVE_PTR<Message> m);

			virtual RR_INTRUSIVE_PTR<MessageEntry> ProcessStreamOpRequest(RR_INTRUSIVE_PTR<MessageEntry> request, RR_INTRUSIVE_PTR<MessageHeader> header);

			virtual RR_SHARED_PTR<RRObject> UnpackStreamOpResponse(RR_INTRUSIVE_PTR<MessageEntry> response, RR_INTRUSIVE_PTR<MessageHeader> header);

			virtual void async_write_some(const_buffers& b, boost::function<void(const boost::system::error_code& error, size_t bytes_transferred)>& handler) = 0;

			virtual void async_read_some(mutable_buffers& b, boost::function<void(const boost::system::error_code& error, size_t bytes_transferred)>& handler) = 0;

			virtual size_t available() = 0;

			virtual bool IsLargeTransferAuthorized();
			
		public:

			virtual bool GetDisableMessage3();
			virtual void SetDisableMessage3(bool d);

			virtual bool CheckCapabilityActive(uint32_t flag);

		};

	}
}
