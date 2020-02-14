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


#include "RobotRaconteur/DataTypes.h"
#include "RobotRaconteur/Message.h"
#include "RobotRaconteur/RobotRaconteurNode.h"
#include "RobotRaconteur/AsyncUtils.h"
#include <boost/call_traits.hpp>


#pragma warning (push)
#pragma warning(disable: 4250)
#pragma warning(disable: 4996)
#include <boost/signals2.hpp>

namespace RobotRaconteur
{
	class ROBOTRACONTEUR_CORE_API  WireBase;
	class ROBOTRACONTEUR_CORE_API  WireConnectionBase;
	class ROBOTRACONTEUR_CORE_API WireConnectionBaseListener;
	namespace detail { class WireSubscription_connection;  }

	class ROBOTRACONTEUR_CORE_API  WireConnectionBase : public RR_ENABLE_SHARED_FROM_THIS<WireConnectionBase>, private boost::noncopyable
	{

		friend class WireBase;
		friend class WireClientBase;
		friend class WireSubscriptionBase;
		friend class detail::WireSubscription_connection;

	public:
		virtual uint32_t GetEndpoint();
		
		virtual TimeSpec GetLastValueReceivedTime();

		virtual TimeSpec GetLastValueSentTime();
		
		virtual void AsyncClose(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout);

		WireConnectionBase(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint = 0, MemberDefinition_Direction direction = MemberDefinition_Direction_both, bool message3 = false);

		virtual ~WireConnectionBase() {}

		virtual void WirePacketReceived(TimeSpec timespec, RR_INTRUSIVE_PTR<RRValue> packet);

		virtual bool GetInValueValid();

		virtual bool GetOutValueValid();
		
		RR_SHARED_PTR<RobotRaconteurNode> GetNode();

		virtual bool GetIgnoreInValue();
		virtual void SetIgnoreInValue(bool ignore);
				
		virtual void AddListener(RR_SHARED_PTR<WireConnectionBaseListener> listener);

		MemberDefinition_Direction Direction();

	protected:

		virtual void RemoteClose();

		RR_INTRUSIVE_PTR<RRValue> inval;
		RR_INTRUSIVE_PTR<RRValue> outval;

		bool inval_valid;
		TimeSpec lasttime_send;

		bool  outval_valid;
		TimeSpec lasttime_recv;

		uint32_t endpoint;
		RR_WEAK_PTR<WireBase> parent;

		bool send_closed;
		bool recv_closed;

		RR_INTRUSIVE_PTR<RRValue> GetInValueBase();

		RR_INTRUSIVE_PTR<RRValue> GetOutValueBase();

		void SetOutValueBase(RR_INTRUSIVE_PTR<RRValue> value);
		
		bool TryGetInValueBase(RR_INTRUSIVE_PTR<RRValue>& value, TimeSpec& time);
		bool TryGetOutValueBase(RR_INTRUSIVE_PTR<RRValue>& value, TimeSpec& time);
		
		virtual void fire_WireValueChanged(RR_INTRUSIVE_PTR<RRValue> value, TimeSpec time)=0;
	
		virtual void fire_WireClosedCallback()=0;

		void Shutdown();
	

		RR_SHARED_PTR<WireBase> GetParent();

		bool ignore_inval;

		bool message3;
		std::list<RR_WEAK_PTR<WireConnectionBaseListener> > listeners;

		RR_WEAK_PTR<RobotRaconteurNode> node;

		MemberDefinition_Direction direction;
		
	};

	template <typename T>
	class WireConnection : public WireConnectionBase
	{
	private:
		boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> WireConnectionClosedCallback;

	public:
		boost::signals2::signal<void (RR_SHARED_PTR<WireConnection<T> > connection, T value, TimeSpec time)> WireValueChanged;
		boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> GetWireConnectionClosedCallback()
		{
			return WireConnectionClosedCallback;
		}

		void SetWireConnectionClosedCallback(boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> callback)
		{
			WireConnectionClosedCallback=callback;
		}

		virtual ~WireConnection() {}

		virtual T GetInValue()
		{
			return RRPrimUtil<T>::PreUnpack(GetInValueBase());
		}

		virtual T GetOutValue()
		{
			return RRPrimUtil<T>::PreUnpack(GetOutValueBase());
		}

		virtual void SetOutValue(typename boost::call_traits<T>::param_type value)
		{
			SetOutValueBase(RRPrimUtil<T>::PrePack(value));
		}

		bool TryGetInValue(T& value, TimeSpec& time)
		{
			RR_INTRUSIVE_PTR<RRValue> o;
			if (!TryGetInValueBase(o, time)) return false;
			value = RRPrimUtil<T>::PreUnpack(o);
			return true;
		}

		bool TryGetOutValue(T& value, TimeSpec& time)
		{
			RR_INTRUSIVE_PTR<RRValue> o;
			if (!TryGetOutValueBase(o, time)) return false;
			value = RRPrimUtil<T>::PreUnpack(o);
			return true;
		}

		WireConnection(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint=0, MemberDefinition_Direction direction = MemberDefinition_Direction_both, bool message3=false)
			: WireConnectionBase(parent,endpoint,direction,message3) {}

	protected:
		virtual void fire_WireValueChanged(RR_INTRUSIVE_PTR<RRValue> value, TimeSpec time)
		{
			WireValueChanged(RR_STATIC_POINTER_CAST<WireConnection<T> >(shared_from_this()),RRPrimUtil<T>::PreUnpack(value),time);
		}
	
		virtual void fire_WireClosedCallback()
		{
			boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> c=GetWireConnectionClosedCallback();
			if (!c) return;
			c(RR_STATIC_POINTER_CAST<WireConnection<T> >(shared_from_this()));
		}

	public:
		/*virtual void Close()
		{
			WireConnectionBase::Close();
			{
				WireConnectionClosedCallback.clear();
			}
			WireValueChanged.disconnect_all_slots();
		}*/

	protected:

		virtual void AsyncClose1(RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler)
		{
			try
			{
				{
					WireConnectionClosedCallback.clear();
				}
				WireValueChanged.disconnect_all_slots();
			}
			catch (std::exception&) {}

			
			handler(err);
		}

	public:
		virtual void AsyncClose(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout=2000)
		{
			WireConnectionBase::AsyncClose(boost::bind(&WireConnection<T>::AsyncClose1,RR_STATIC_POINTER_CAST<WireConnection<T> >(shared_from_this()),_1,handler),timeout);
			
		}

	protected:
		virtual void RemoteClose()
		{
			WireConnectionBase::RemoteClose();
			{
				WireConnectionClosedCallback.clear();
			}
			WireValueChanged.disconnect_all_slots();
		}


	};


	class ROBOTRACONTEUR_CORE_API  WireBase : public RR_ENABLE_SHARED_FROM_THIS<WireBase>, private boost::noncopyable
	{
		

	public:

		friend class WireConnectionBase;
		
		virtual ~WireBase() {}

		virtual std::string GetMemberName()=0;

		virtual void WirePacketReceived(RR_INTRUSIVE_PTR<MessageEntry> m, uint32_t e=0)=0;

		virtual void Shutdown()=0;


		

		virtual void AsyncClose(RR_SHARED_PTR<WireConnectionBase> endpoint, bool remote, uint32_t ee, RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)=0;

	protected:

		virtual void SendWirePacket(RR_INTRUSIVE_PTR<RRValue> data, TimeSpec time, uint32_t endpoint, bool message3)=0;

		bool rawelements;		

		void DispatchPacket (RR_INTRUSIVE_PTR<MessageEntry> me, RR_SHARED_PTR<WireConnectionBase> e);

		RR_INTRUSIVE_PTR<RRValue> UnpackPacket(RR_INTRUSIVE_PTR<MessageEntry> me, TimeSpec& ts);

		RR_INTRUSIVE_PTR<MessageEntry> PackPacket(RR_INTRUSIVE_PTR<RRValue> data, TimeSpec time, bool message3);

		virtual RR_INTRUSIVE_PTR<MessageElementData> PackData(RR_INTRUSIVE_PTR<RRValue> data)
		{
			return GetNode()->PackVarType(data);
		}

		virtual RR_INTRUSIVE_PTR<RRValue> UnpackData(RR_INTRUSIVE_PTR<MessageElement> mdata)
		{
			return GetNode()->UnpackVarType(mdata);
		}

		
		RR_WEAK_PTR<RobotRaconteurNode> node;

		MemberDefinition_Direction direction;

	public:

		RR_SHARED_PTR<RobotRaconteurNode> GetNode();

		MemberDefinition_Direction Direction();	

	};


	template <typename T>
	class Wire : public virtual WireBase
	{

		friend class WireConnectionBase;

	public:

		Wire(boost::function<void(RR_INTRUSIVE_PTR<RRValue>&)> verify)
		{
			this->verify = verify;
		}

		virtual ~Wire() {}
		
		//Client side functions
		virtual RR_SHARED_PTR<WireConnection<T> > Connect() = 0;
		virtual void AsyncConnect(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<WireConnection<T> >, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;

		virtual void AsyncPeekInValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		virtual void AsyncPeekOutValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		virtual void AsyncPokeOutValue(const T& value, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
	protected:

		virtual RR_INTRUSIVE_PTR<MessageElementData> PackData(RR_INTRUSIVE_PTR<RRValue> data)
		{
			if (verify)
			{
				verify(data);
			}
			return GetNode()->template PackAnyType<typename RRPrimUtil<T>::BoxedType>(data);
		}

		virtual RR_INTRUSIVE_PTR<RRValue> UnpackData(RR_INTRUSIVE_PTR<MessageElement> mdata)
		{
			if (!verify)
			{
				return GetNode()->template UnpackAnyType<typename RRPrimUtil<T>::BoxedType>(mdata);
			}
			else
			{
				RR_INTRUSIVE_PTR<RRValue> ret = GetNode()->template UnpackAnyType<typename RRPrimUtil<T>::BoxedType>(mdata);
				verify(ret);
				return ret;
			}
		}

		boost::function<void(RR_INTRUSIVE_PTR<RRValue>&)> verify;
	};


	class ROBOTRACONTEUR_CORE_API  ServiceStub;
	
	class ROBOTRACONTEUR_CORE_API  WireClientBase : public virtual WireBase
	{
		friend class WireConnectionBase;
		friend class WireSubscriptionBase;

	public:

		virtual ~WireClientBase() {}

		virtual std::string GetMemberName();

		virtual void WirePacketReceived(RR_INTRUSIVE_PTR<MessageEntry> m, uint32_t e = 0);

		virtual void Shutdown();

		virtual void AsyncClose(RR_SHARED_PTR<WireConnectionBase> endpoint, bool remote, uint32_t ee, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout);

		RR_SHARED_PTR<ServiceStub> GetStub();

	protected:

		virtual void SendWirePacket(RR_INTRUSIVE_PTR<RRValue> packet, TimeSpec time, uint32_t endpoint, bool message3);

		std::string m_MemberName;

		RR_SHARED_PTR<WireConnectionBase> connection;

		RR_WEAK_PTR<ServiceStub> stub;

		void AsyncConnect_internal(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<WireConnectionBase>, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout);

		void AsyncConnect_internal1(RR_INTRUSIVE_PTR<MessageEntry> ret, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_SHARED_PTR<WireConnectionBase>, RR_SHARED_PTR<RobotRaconteurException>)>& handler);


		WireClientBase(boost::string_ref name, RR_SHARED_PTR<ServiceStub> stub, MemberDefinition_Direction direction);

		virtual RR_SHARED_PTR<WireConnectionBase> CreateNewWireConnection(MemberDefinition_Direction direction, bool message3) = 0;

		void AsyncPeekInValueBase(RR_MOVE_ARG(boost::function<void(const RR_INTRUSIVE_PTR<RRValue>&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE);
		void AsyncPeekOutValueBase(RR_MOVE_ARG(boost::function<void(const RR_INTRUSIVE_PTR<RRValue>&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE);
		void AsyncPokeOutValueBase(const RR_INTRUSIVE_PTR<RRValue>& value, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE);

		void AsyncPeekValueBaseEnd1(RR_INTRUSIVE_PTR<MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err,
			boost::function< void(const RR_INTRUSIVE_PTR<RRValue>&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>) >& handler);
				
	};

	template <typename T>
	class WireClient : public virtual Wire<T>, public virtual WireClientBase
	{
	public:

		WireClient(boost::string_ref name, RR_SHARED_PTR<ServiceStub> stub, MemberDefinition_Direction direction = MemberDefinition_Direction_both, boost::function<void(RR_INTRUSIVE_PTR<RRValue>&)> verify = NULL) : WireClientBase(name, stub, direction), Wire<T>(verify)
		{
			if (boost::is_same<T, RR_INTRUSIVE_PTR<MessageElement> >::value)
			{
				rawelements = true;
			}
			else
			{
				rawelements = false;
			}

		}

		virtual ~WireClient() {}
				
		virtual void AsyncConnect(RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<WireConnection<T> >, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout=RR_TIMEOUT_INFINITE)
		{
			AsyncConnect_internal(boost::bind(handler,boost::bind(&WireClient<T>::AsyncConnect_cast,_1),_2),timeout);
		}
		
	protected:

		static RR_SHARED_PTR<WireConnection<T> > AsyncConnect_cast(RR_SHARED_PTR<WireConnectionBase> b)
		{
			return rr_cast<WireConnection<T> >(b);
		}

		void AsyncPeekValueBaseEnd2(const RR_INTRUSIVE_PTR<RRValue>& value, const TimeSpec& ts, RR_SHARED_PTR<RobotRaconteurException> err,
			boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>& handler)
		{			

			if (err)
			{
				typename boost::initialized<T> err_value;
				handler(err_value, ts, err);
				return;
			}
			
			T value2;
			try
			{
				value2 = RRPrimUtil<T>::PreUnpack(value);				
			}
			catch (std::exception& exp)
			{
				typename boost::initialized<T> err_value;
				RR_SHARED_PTR<RobotRaconteurException> err = RobotRaconteurExceptionUtil::ExceptionToSharedPtr(exp);
				handler(err_value, ts, err);
				return;
			}

			handler(value2, ts, err);
		}

	public:
		
		virtual void AsyncPeekInValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			AsyncPeekInValueBase(boost::bind(&WireClient::AsyncPeekValueBaseEnd2, RR_DYNAMIC_POINTER_CAST<WireClient>(shared_from_this()), _1, _2, _3, RR_MOVE(handler)), timeout);
		}
		virtual void AsyncPeekOutValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			AsyncPeekOutValueBase(boost::bind(&WireClient::AsyncPeekValueBaseEnd2, RR_DYNAMIC_POINTER_CAST<WireClient>(shared_from_this()), _1, _2, _3, RR_MOVE(handler)), timeout);
		}
		virtual void AsyncPokeOutValue(const T& value, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			AsyncPokeOutValueBase(RRPrimUtil<T>::PrePack(value), RR_MOVE(handler), timeout);
		}
		
	protected:
		virtual RR_SHARED_PTR<WireConnectionBase> CreateNewWireConnection(MemberDefinition_Direction direction, bool message3)
		{
			return RR_MAKE_SHARED<WireConnection<T> >(RR_STATIC_POINTER_CAST<WireBase>(shared_from_this()), 0, direction, message3);
		}

	};

#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES
	using WireConnectionBasePtr = RR_SHARED_PTR<WireConnectionBase>;
	template <typename T> using WireConnectionPtr = RR_SHARED_PTR<WireConnection<T> >;
	using WireBasePtr = RR_SHARED_PTR<WireBase>;
	template <typename T> using WirePtr = RR_SHARED_PTR<Wire<T> >;	
#endif

}

#pragma warning(pop)
