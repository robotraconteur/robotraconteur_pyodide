// Copyright 2011-2020 Wason Technology, LLC
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

#include "RobotRaconteur/WireMember.h"
#include "RobotRaconteur/Client.h"
#include "RobotRaconteur/DataTypes.h"

#include "WireMember_private.h"

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

#define RR_WIRE_CONNECTION_LISTENER_ITER(command) \
	try \
	{ \
		for (std::list<RR_WEAK_PTR<WireConnectionBaseListener> >::iterator e = listeners.begin(); e != listeners.end();) \
		{ \
			RR_SHARED_PTR<WireConnectionBaseListener> w1 = e->lock(); \
			if (!w1) \
			{ \
				e = listeners.erase(e); \
				continue; \
			} \
			command; \
			e++; \
		} \
	} \
	catch (std::exception& exp) \
	{ \
		RobotRaconteurNode::TryHandleException(node, &exp); \
	} \

namespace RobotRaconteur
{	
	uint32_t WireConnectionBase::GetEndpoint()
	{
		return endpoint;
	}

	TimeSpec WireConnectionBase::GetLastValueReceivedTime()
	{
		if (!inval_valid)
			throw ValueNotSetException("No value received");
		return lasttime_recv;
	}

	TimeSpec WireConnectionBase::GetLastValueSentTime()
	{
		if (!outval_valid)
			throw ValueNotSetException("No value sent");
		return lasttime_send;
	}

	RR_SHARED_PTR<WireBase> WireConnectionBase::GetParent()
	{
		RR_SHARED_PTR<WireBase> out=parent.lock();
		if (!out) throw InvalidOperationException("Wire connection has been closed");
		return out;
	}

	void WireConnectionBase::AsyncClose(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Requesting close wire connection");

		{
			send_closed = true;
			
			GetParent()->AsyncClose(shared_from_this(), false, endpoint, RR_MOVE(handler), timeout);
		}

		{
			recv_closed = true;
		}		
	}

	WireConnectionBase::WireConnectionBase(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint, MemberDefinition_Direction direction)
	{
		this->parent=parent;
		this->endpoint=endpoint;
		outval_valid=false;
		inval_valid=false;
		inval_lifespan=-1;
		outval_lifespan=-1;
		ignore_inval = false;
		send_closed = false;
		recv_closed = false;
		node = parent->GetNode();
		this->direction = direction;
		this->service_path=parent->GetServicePath();
		this->member_name=parent->GetMemberName();
	}

	RR_SHARED_PTR<RobotRaconteurNode> WireConnectionBase::GetNode()
	{
		RR_SHARED_PTR<RobotRaconteurNode> n = node.lock();
		if (!n) throw InvalidOperationException("Node has been released");
		return n;
	}

	void WireConnectionBase::WirePacketReceived(TimeSpec timespec, RR_INTRUSIVE_PTR<RRValue> packet)
	{		
		{
			
			RR_SHARED_PTR<RobotRaconteurNode> n = node.lock();

			if (ignore_inval)
			{
				return;
			}

			if (lasttime_recv == TimeSpec(0,0) || timespec > lasttime_recv)
			{
				ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Wire packet in value received timespec " << timespec.seconds << "," << timespec.nanoseconds);

				{
					inval = packet;
					lasttime_recv = timespec;
					if (n)
					{
						lasttime_recv_local = n->NowUTC();
					}					
					inval_valid = true;
					
				}

				
				RR_WIRE_CONNECTION_LISTENER_ITER(w1->WireValueChanged(shared_from_this(), packet, timespec));
				
				try
				{
					fire_WireValueChanged(packet, timespec);
				}
				catch (std::exception& exp)
				{
					RobotRaconteurNode::TryHandleException(node, &exp);
				}

			}
			else
			{
				ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Old wire packet received timespec " << timespec.seconds << "," << timespec.nanoseconds << ", dropping");
			}
			
		}
	}

	void WireConnectionBase_RemoteClose_emptyhandler(RR_SHARED_PTR<RobotRaconteurException> err) {}

	void WireConnectionBase::RemoteClose()
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Received remote close wire connection");

		{
			send_closed = true;			
		}

		{
			recv_closed = true;			
		}

		try
		{
			fire_WireClosedCallback();
		}
		catch (std::exception& exp)
		{
			RobotRaconteurNode::TryHandleException(node, &exp);
		}

		
		RR_WIRE_CONNECTION_LISTENER_ITER(w1->WireConnectionClosed(shared_from_this()));
		
		try
		{
			//if (parent.expired()) return;
			//boost::mutex::scoped_lock lock2 (recvlock);
			GetParent()->AsyncClose(shared_from_this(),true,endpoint,&WireConnectionBase_RemoteClose_emptyhandler,1000);
		}
		catch (std::exception&) {}
		
	}

	namespace detail
	{
		bool WireConnectionBase_IsValueExpired(RR_WEAK_PTR<RobotRaconteurNode> node, const boost::posix_time::ptime& recv_time, int32_t lifespan)
		{
			if (lifespan < 0)
			{
				return false;
			}

			RR_SHARED_PTR<RobotRaconteurNode> n = node.lock();
			if (!n)
			{
				return true;
			}

			if (recv_time + boost::posix_time::milliseconds(lifespan) < n->NowUTC())
			{
				return true;
			}
			return false;
		}
	}

	RR_INTRUSIVE_PTR<RRValue> WireConnectionBase::GetInValueBase()
	{
		if (direction == MemberDefinition_Direction_writeonly)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Attempt to get InValue of write only wire");
			throw WriteOnlyMemberException("Write only member");
		}
		RR_INTRUSIVE_PTR<RRValue> val;
		{
		if (!inval_valid)
			throw ValueNotSetException("Value not set");
		if(detail::WireConnectionBase_IsValueExpired(node, lasttime_recv_local,inval_lifespan))
		{
			throw ValueNotSetException("Value expired");
		}
		val=inval;
		}
		return val;
	}

	RR_INTRUSIVE_PTR<RRValue> WireConnectionBase::GetOutValueBase() 
	{
		if (direction == MemberDefinition_Direction_readonly)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Attempt to get OutValue of read only wire");
			throw ReadOnlyMemberException("Read only member");
		}
		RR_INTRUSIVE_PTR<RRValue> val;
		{
		if (!outval_valid)
			throw ValueNotSetException("Value not set");
		if(detail::WireConnectionBase_IsValueExpired(node, lasttime_send_local,outval_lifespan))
		{
			throw ValueNotSetException("Value expired");
		}
		val=outval;
		}
		return val;
	}

	void WireConnectionBase::SetOutValueBase(RR_INTRUSIVE_PTR<RRValue> value)
	{
		if (direction == MemberDefinition_Direction_readonly)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Attempt to get OutValue of read only wire");
			throw ReadOnlyMemberException("Read only member");
		}

		RR_SHARED_PTR<RobotRaconteurNode> n = node.lock();

		try		
		{
			
			TimeSpec time = TimeSpec::Now();
			if (time <= lasttime_send)
			{
				time=lasttime_send;
				time.nanoseconds += 1;
				time.cleanup_nanosecs();
			}
			
			ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Wire sending out value packet timespec " << time.seconds << "," << time.nanoseconds);
			
			GetParent()->SendWirePacket(value, time, endpoint);
			
			
			outval = value;
			lasttime_send = time;
			lasttime_send_local = n->NowUTC();
			outval_valid = true;
			
			
		}
		catch (std::exception& exp)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Wire set OutValue failed: " << exp.what());
			throw;
		}
	}

	bool WireConnectionBase::TryGetInValueBase(RR_INTRUSIVE_PTR<RRValue>& value, TimeSpec& time)
	{
		if (direction == MemberDefinition_Direction_writeonly)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Attempt to get InValue of write only wire");
			throw WriteOnlyMemberException("Write only member");
		}
		
		if (!inval_valid) return false;
		if(detail::WireConnectionBase_IsValueExpired(node, lasttime_recv_local,inval_lifespan))
		{
			return false;
		}
		value = inval;
		time = lasttime_recv;
		return true;
	}

	bool WireConnectionBase::TryGetOutValueBase(RR_INTRUSIVE_PTR<RRValue>& value, TimeSpec& time)
	{
		if (direction == MemberDefinition_Direction_readonly)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "Attempt to get OutValue of read only wire");
			throw ReadOnlyMemberException("Read only member");
		}
		
		if (!outval_valid) return false;
		if(detail::WireConnectionBase_IsValueExpired(node, lasttime_send_local,outval_lifespan))
		{
			return false;
		}			
		value = outval;
		time = lasttime_send;
		return true;
	}

	bool WireConnectionBase::GetInValueValid()
	{
		return inval_valid;
	}

	bool WireConnectionBase::GetOutValueValid()
	{
		return outval_valid;
	}

	bool WireConnectionBase::GetIgnoreInValue()
	{
		return ignore_inval;
	}

	void WireConnectionBase::SetIgnoreInValue(bool ignore)
	{
		ignore_inval = ignore;
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "IgnoreInValue set to " << ignore)
	}

	void WireConnectionBase::AddListener(RR_SHARED_PTR<WireConnectionBaseListener> listener)
	{
		listeners.push_back(listener);
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "WireConnectionBaseListener added to wire connection");
	}

	void WireConnectionBase::Shutdown()
	{
		{
			send_closed = true;			
		}

		{
			recv_closed = true;			
		}

		RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(&WireConnectionBase::fire_WireClosedCallback, shared_from_this()),true);

		std::list<RR_WEAK_PTR<WireConnectionBaseListener> > listeners1;
		{
			listeners.swap(listeners1);
		}
		
		BOOST_FOREACH(RR_WEAK_PTR<WireConnectionBaseListener> l, listeners1)
		{
			RR_SHARED_PTR<WireConnectionBaseListener> l1 = l.lock();
			if (l1)
			{
				RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(&WireConnectionBaseListener::WireConnectionClosed, l1, shared_from_this()),true);
			}
		}

		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, member_name, "WireConnectionBase shut down");
	}

	MemberDefinition_Direction WireConnectionBase::Direction()
	{
		return direction;
	}

	int32_t WireConnectionBase::GetInValueLifespan()
	{
		boost::mutex::scoped_lock lock(inval_lock);
		return inval_lifespan;
	}
	void WireConnectionBase::SetInValueLifespan(int32_t millis)
	{
		boost::mutex::scoped_lock lock(inval_lock);
		inval_lifespan = millis;
	}
	
	int32_t WireConnectionBase::GetOutValueLifespan()
	{
		boost::mutex::scoped_lock lock(outval_lock);
		return outval_lifespan;
	}	
	void WireConnectionBase::SetOutValueLifespan(int32_t millis)
	{
		boost::mutex::scoped_lock lock(outval_lock);
		outval_lifespan = millis;
	}

	RR_INTRUSIVE_PTR<RRValue> WireBase::UnpackPacket(RR_INTRUSIVE_PTR<MessageEntry> me, TimeSpec& ts)
	{
		
		RR_INTRUSIVE_PTR<MessageElementNestedElementList> s = MessageElement::FindElement(me->elements, "packettime")->CastDataToNestedList(DataTypes_structure_t);
		int64_t seconds = RRArrayToScalar(MessageElement::FindElement(s->Elements, "seconds")->CastData<RRArray<int64_t> >());
		int32_t nanoseconds = RRArrayToScalar(MessageElement::FindElement(s->Elements, "nanoseconds")->CastData<RRArray<int32_t> >());
		ts = TimeSpec(seconds, nanoseconds);		

		RR_INTRUSIVE_PTR<RRValue> data;
		if (!rawelements)
		{
			data = UnpackData(MessageElement::FindElement(me->elements, "packet"));
		}
		else
		{
			data = MessageElement::FindElement(me->elements, "packet");
		}

		return data;
	}

	void WireBase::DispatchPacket (RR_INTRUSIVE_PTR<MessageEntry> me, RR_SHARED_PTR<WireConnectionBase> e)
	{
		TimeSpec timespec;
		RR_INTRUSIVE_PTR<RRValue> data = UnpackPacket(me,timespec);
		e->WirePacketReceived(timespec,data);
	}

	RR_INTRUSIVE_PTR<MessageEntry> WireBase::PackPacket(RR_INTRUSIVE_PTR<RRValue> data, TimeSpec time)
	{		
		std::vector<RR_INTRUSIVE_PTR<MessageElement> > timespec1;
		timespec1.push_back(CreateMessageElement("seconds", ScalarToRRArray(time.seconds)));
		timespec1.push_back(CreateMessageElement("nanoseconds", ScalarToRRArray(time.nanoseconds)));
		RR_INTRUSIVE_PTR<MessageElementNestedElementList> s = CreateMessageElementNestedElementList(DataTypes_structure_t, "RobotRaconteur.TimeSpec", RR_MOVE(timespec1));


		std::vector<RR_INTRUSIVE_PTR<MessageElement> > elems;
		elems.push_back(CreateMessageElement("packettime", s));
		if (!rawelements)
		{
			RR_INTRUSIVE_PTR<MessageElementData> pdata = PackData(data);
			elems.push_back(CreateMessageElement("packet", pdata));
		}
		else
		{
			RR_INTRUSIVE_PTR<MessageElement> pme = RR_DYNAMIC_POINTER_CAST<MessageElement>(data);
			pme->ElementName = "packet";
			elems.push_back(pme);
		}

		RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_WirePacket, GetMemberName());
		m->elements = elems;
		m->MetaData = "unreliable\n";
		return m;		
	}

	RR_SHARED_PTR<RobotRaconteurNode> WireBase::GetNode()
	{
		RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
		if (!n) throw InvalidOperationException("Node has been released");
		return n;
	}

	MemberDefinition_Direction WireBase::Direction()
	{
		return direction;
	}

	std::string WireClientBase::GetMemberName()
	{
		return m_MemberName;
	}

	static void empty_close_handler(RR_SHARED_PTR<RobotRaconteurException> err) {}
	std::string WireClientBase::GetServicePath()
	{
		return service_path;
	}

	void WireClientBase::WirePacketReceived(RR_INTRUSIVE_PTR<MessageEntry> m, uint32_t e)
	{
		//boost::shared_lock<boost::shared_mutex> lock2(stub_lock);
		
		if (m->EntryType == MessageEntryType_WireClosed)
			{
				try
				{
					connection->AsyncClose(&empty_close_handler,GetNode()->GetRequestTimeout());
					connection.reset();
				}
				catch (std::exception& exp)
				{
					ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Error closing wire connection: " << exp.what());
				}
			}
			else if (m->EntryType == MessageEntryType_WirePacket)
			{
				try
				{
					RR_SHARED_PTR<WireConnectionBase> c;
					{
					c=connection;
					if (!c)
					{
						ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Received packet for unconnected wire");
						return;
					}
					}
					DispatchPacket(m, c);
				}
				catch (std::exception& exp)
				{
					ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Error receiving and dispatching wire packet: " << exp.what());
				}
			}
	}

	void WireClientBase::Shutdown()
	{
		
		try
		{
			RR_SHARED_PTR<WireConnectionBase> c;
			{
				c=connection;
			}
			
			
			try
			{
				//RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_WireDisconnectReq, GetMemberName());
				//RR_INTRUSIVE_PTR<MessageEntry> ret = GetStub()->ProcessRequest(m);
				if (c)	c->Shutdown();
							
			}
			catch (std::exception& exp) 
			{
				RobotRaconteurNode::TryHandleException(node, &exp);
			}
			
		}
		catch (std::exception&)
		{
		}

		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "WireClient shut down");
		
	}

	RR_SHARED_PTR<ServiceStub> WireClientBase::GetStub()
	{
		RR_SHARED_PTR<ServiceStub> out=stub.lock();
		if (!out) throw InvalidOperationException("Wire connection has been closed");
		return out;
	}

	void WireClientBase::SendWirePacket(RR_INTRUSIVE_PTR<RRValue> packet, TimeSpec time, uint32_t endpoint)
	{
		RR_INTRUSIVE_PTR<MessageEntry> m = PackPacket(packet, time);
		
		GetStub()->SendWireMessage(m);
	}
	
	void WireClientBase::AsyncClose(RR_SHARED_PTR<WireConnectionBase> endpoint, bool remote, uint32_t ee, RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
	{
		
		
		{
			if (!remote)
			{
			RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_WireDisconnectReq, GetMemberName());
			GetStub()->AsyncProcessRequest(m,boost::bind(handler,RR_BOOST_PLACEHOLDERS(_2)),timeout);
			}
			connection.reset();
		}
	}

	void WireClientBase::AsyncConnect_internal(RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<WireConnectionBase>,RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
	{
		
		
		{
			try
			{
				if (connection != 0)
				{
					ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Wire already connected");
					throw InvalidOperationException("Already connected");
				}
				
				RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_WireConnectReq, GetMemberName());
				GetStub()->AsyncProcessRequest(m,boost::bind(&WireClientBase::AsyncConnect_internal1, RR_DYNAMIC_POINTER_CAST<WireClientBase>(shared_from_this()),RR_BOOST_PLACEHOLDERS(_1),RR_BOOST_PLACEHOLDERS(_2),handler),timeout);

				ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Begin connect wire connection");
				
			}
			catch (std::exception &e)
			{
				connection.reset();
				ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Wire connect failed: " << e.what());
				throw e;
			}
		}
	}

	void WireClientBase::AsyncConnect_internal1(RR_INTRUSIVE_PTR<MessageEntry> ret, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void (RR_SHARED_PTR<WireConnectionBase>,RR_SHARED_PTR<RobotRaconteurException>)>& handler)
	{
		if (err)
		{
			ROBOTRACONTEUR_LOG_DEBUG_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Connecting wire failed: " << err->what());
			detail::InvokeHandlerWithException(node, handler,err);
			return;			
		}

		try
		{		
			{
				if (connection)
				{
					
					detail::InvokeHandlerWithException(node, handler, RR_MAKE_SHARED<ServiceException>("Wire already connected"));
					
					return;
				}
				connection = CreateNewWireConnection(direction);
			}
			
			ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Wire connected");

			detail::InvokeHandler(node, handler, connection);
			
			
		}
		catch (std::exception& err2)
		{
			ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Connecting wire failed: " << err2.what());
			detail::InvokeHandlerWithException(node, handler, err2);		
		}
	}

	WireClientBase::WireClientBase(boost::string_ref name, RR_SHARED_PTR<ServiceStub> stub, MemberDefinition_Direction direction)
	{
		this->stub=stub;
		this->m_MemberName = RR_MOVE(name.to_string());
		this->node=stub->RRGetNode();
		this->direction = direction;
		this->service_path = stub->ServicePath;
		this->endpoint=stub->GetContext()->GetLocalEndpoint();

		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "WireClient created");
	}


	void WireClientBase::AsyncPeekValueBaseEnd1(RR_INTRUSIVE_PTR<MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, 
		boost::function< void(const RR_INTRUSIVE_PTR<RRValue>&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>) >& handler)
	{
		TimeSpec ts;
		RR_INTRUSIVE_PTR<RRValue> value;
		if (err)
		{			
			handler(RR_INTRUSIVE_PTR<RRValue>(), ts, err);
			return;
		}
		if (m->Error != RobotRaconteur::MessageErrorType_None)
		{			
			handler(RR_INTRUSIVE_PTR<RRValue>(), ts, RobotRaconteurExceptionUtil::MessageEntryToException(m));
			return;
		}		
		try
		{			
			value = UnpackPacket(m, ts);
		}
		catch (RobotRaconteur::RobotRaconteurException& err)
		{			
			handler(RR_INTRUSIVE_PTR<RRValue>(), ts, RobotRaconteur::RobotRaconteurExceptionUtil::DownCastException(err));
			return;
		}
		catch (std::exception& err)
		{			
			handler(RR_INTRUSIVE_PTR<RRValue>(), ts, 
				RR_MAKE_SHARED<RobotRaconteurRemoteException>(std::string(typeid(err).name()), err.what()));
			return;
		}
		handler(value, ts, RR_SHARED_PTR<RobotRaconteur::RobotRaconteurException>());
	}

	void WireClientBase::AsyncPeekInValueBase(RR_MOVE_ARG(boost::function<void(const RR_INTRUSIVE_PTR<RRValue>&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Requesting PeekInValue");
		RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m = CreateMessageEntry(MessageEntryType_WirePeekInValueReq, GetMemberName());
		GetStub()->AsyncProcessRequest(m, boost::bind(&WireClientBase::AsyncPeekValueBaseEnd1, RR_DYNAMIC_POINTER_CAST<WireClientBase>(shared_from_this()), RR_BOOST_PLACEHOLDERS(_1), RR_BOOST_PLACEHOLDERS(_2), handler),timeout);		
	}

	void WireClientBase::AsyncPeekOutValueBase(RR_MOVE_ARG(boost::function<void(const RR_INTRUSIVE_PTR<RRValue>&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Requesting PeekOutValue");
		RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m = CreateMessageEntry(MessageEntryType_WirePeekOutValueReq, GetMemberName());
		GetStub()->AsyncProcessRequest(m, boost::bind(&WireClientBase::AsyncPeekValueBaseEnd1, RR_DYNAMIC_POINTER_CAST<WireClientBase>(shared_from_this()), RR_BOOST_PLACEHOLDERS(_1), RR_BOOST_PLACEHOLDERS(_2), handler), timeout);
	}

	void WireClientBase_AsyncPokeValueBaseEnd(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteur::RobotRaconteurException> err, boost::function< void(RR_SHARED_PTR<RobotRaconteur::RobotRaconteurException>) > handler)
	{
		if (err)
		{
			handler(err);
			return;
		}
		if (m->Error != RobotRaconteur::MessageErrorType_None)
		{
			handler(RobotRaconteur::RobotRaconteurExceptionUtil::MessageEntryToException(m));
			return;
		}
		handler(RR_SHARED_PTR<RobotRaconteur::RobotRaconteurException>());
	}

	void WireClientBase::AsyncPokeOutValueBase(const RR_INTRUSIVE_PTR<RRValue>& value, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
	{
		ROBOTRACONTEUR_LOG_TRACE_COMPONENT_PATH(node, Member, endpoint, service_path, m_MemberName, "Requesting PokeOutValue");
		RR_INTRUSIVE_PTR<MessageEntry> m = PackPacket(value, TimeSpec::Now());
		m->EntryType = MessageEntryType_WirePokeOutValueReq;
		m->MetaData.reset();
		GetStub()->AsyncProcessRequest(m, boost::bind(&WireClientBase_AsyncPokeValueBaseEnd, RR_BOOST_PLACEHOLDERS(_1), RR_BOOST_PLACEHOLDERS(_2), handler), timeout);
	}


	
}