/** 
 * @file WireMember.h
 * 
 * @author John Wason, PhD
 * 
 * @copyright Copyright 2011-2020 Wason Technology, LLC
 *
 * @par License
 * Software License Agreement (Apache License)
 * @par
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * @par
 * http://www.apache.org/licenses/LICENSE-2.0
 * @par
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
	namespace detail 
	{ 
		class WireSubscription_connection;  
		bool WireConnectionBase_IsValueExpired(RR_WEAK_PTR<RobotRaconteurNode> node, const boost::posix_time::ptime& recv_time, int32_t lifespan);		
	}

	/**
	 * @brief Base class for WireConnection
	 * 
	 * Base class for templated WireConnection
	 * 
	 */
	class ROBOTRACONTEUR_CORE_API  WireConnectionBase : public RR_ENABLE_SHARED_FROM_THIS<WireConnectionBase>, private boost::noncopyable
	{

		friend class WireBase;
		friend class WireClientBase;
		friend class WireSubscriptionBase;
		friend class detail::WireSubscription_connection;

	public:

		/**
		 * @brief Returns the Robot Raconteur node Endpoint ID
		 * 
		 * Returns the endpoint associated with the ClientContext or ServerEndpoint
		 * associated with the wire connection.
		 * 
		 * @return uint32_t The Robot Raconteur node Endpoint ID
		 */
		virtual uint32_t GetEndpoint();
		
		/**
		 * @brief Get the timestamp of the last received value
		 * 
		 * Returns the timestamp of the value in the *senders* clock
		 * 
		 * @return TimeSpec The timestamp of the last received value
		 */
		virtual TimeSpec GetLastValueReceivedTime();

		/**
		 * @brief Get the timestamp of the last sent value
		 * 
		 * Returns the timestamp of the last sent value in the *local* clock
		 * 
		 * @return TimeSpec The timestamp of the last sent value
		 */
		virtual TimeSpec GetLastValueSentTime();
		

		/**
		 * @brief Asynchronously close the wire connection
		 * 
		 * Same as Close() but returns asynchronously
		 * 
		 * @param handler A handler function to call on completion, possibly with an exception
		 * @param timeout Timeout in milliseconds, or RR_TIMEOUT_INFINITE for no timeout
		 */
		virtual void AsyncClose(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout);

		WireConnectionBase(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint = 0, MemberDefinition_Direction direction = MemberDefinition_Direction_both);

		virtual ~WireConnectionBase() {}

		virtual void WirePacketReceived(TimeSpec timespec, RR_INTRUSIVE_PTR<RRValue> packet);

		/**
		 * @brief Get if the InValue is valid
		 * 
		 * The InValue is valid if a value has been received and 
		 * the value has not expired
		 * 
		 * @return true The InValue is valid
		 * @return false The OutValue is invalid
		 */
		virtual bool GetInValueValid();

		/**
		 * @brief Get if the OutValue is valid
		 * 
		 * The OutValue is valid if a value has been
		 * set using SetOutValue()
		 * 
		 * @return true The OutValue is valid
		 * @return false The OutValue is invalid
		 */
		virtual bool GetOutValueValid();
		
		RR_SHARED_PTR<RobotRaconteurNode> GetNode();

		/**
		 * @brief Get if wire connection is ignoring incoming values
		 * 
		 * If true, wire connection is ignoring incoming values and is not
		 * storing the value
		 * 
		 * @return true Wire connection is ignoring incoming values
		 * @return false Wire connection is not ignoring incoming values
		 */
		virtual bool GetIgnoreInValue();

		/**
		 * @brief Set whether wire connection should ignore incoming values
		 * 
		 * Wire connections may optionally desire to ignore incoming values. This is useful if the connection
		 * is only being used to send out values, and received values may create a potential memory . If ignore is true, 
		 * incoming values will be discarded.
		 * 
		 * @param ignore If true, incoming values are ignored. If false, the most recent value is stored.
		 */
		virtual void SetIgnoreInValue(bool ignore);
				
		virtual void AddListener(RR_SHARED_PTR<WireConnectionBaseListener> listener);

		/**
		 * @brief The direction of the wire
		 * 
		 * Wires may be declared *readonly* or *writeonly* in the service definition file. (If neither
		 * is specified, the wire is assumed to be full duplex.) *readonly* wire may only send out values from
		 * service to client. *writeonly* wires may only send out values from client to service.
		 * 
		 * @return MemberDefinition_Direction 
		 */
		MemberDefinition_Direction Direction();

		/**
		 * @brief Get the lifespan of InValue
		 * 
		 * InValue may optionally have a finite lifespan specified in milliseconds.
		 * Once the lifespan after reception has expired, the InValue is cleared, and becomes invalid.
		 * Attempts to access InValue will result in a ValueNotSetException.
		 * 
		 * @return int32_t The lifespan in milliseconds
		 */
		virtual int32_t GetInValueLifespan();
		/**
		 * @brief Set the lifespan of InValue
		 * 
		 * InValue may optionally have a finite lifespan specified in milliseconds. Once
		 * the lifespan after reception has expired, the InValue is cleared and becomes invalid.
		 * Attempts to access InValue will result in ValueNotSetException.
		 * 
		 * InValue lifespans may be used to avoid using a stale value received by the wire. If
		 * the lifespan is not set, the wire will continue to return the last received value, even
		 * if the value is old.
		 * 
		 * @param millis The lifespan in millisecond, or RR_VALUE_LIFESPAN_INFINITE for infinite lifespan
		 */
		virtual void SetInValueLifespan(int32_t millis);
		
		/**
		 * @brief Get the lifespan of OutValue
		 * 
		 * OutValue may optionally have a finite lifespan specified in milliseconds.
		 * Once the lifespan after sending has expired, the OutValue is cleared, and becomes invalid.
		 * Attempts to access OutValue will result in a ValueNotSetException.
		 * 
		 * @return int32_t The lifespan in milliseconds
		 */
		virtual int32_t GetOutValueLifespan();
		/**
		 * @brief Set the lifespan of OutValue
		 * 
		 * OutValue may optionally have a finite lifespan specified in milliseconds. Once
		 * the lifespan after sending has expired, the OutValue is cleared and becomes invalid.
		 * Attempts to access OutValue will result in ValueNotSetException.
		 * 
		 * OutValue lifespans may be used to avoid using a stale value sent by the wire. If
		 * the lifespan is not set, the wire will continue to return the last sent value, even
		 * if the value is old.
		 * 
		 * @param millis The lifespan in millisecond, or RR_VALUE_LIFESPAN_INFINITE for infinite lifespan
		 */
		virtual void SetOutValueLifespan(int32_t millis);
		

	protected:

		virtual void RemoteClose();

		RR_INTRUSIVE_PTR<RRValue> inval;
		RR_INTRUSIVE_PTR<RRValue> outval;

		bool inval_valid;
		TimeSpec lasttime_send;
		boost::posix_time::ptime lasttime_send_local;

		bool  outval_valid;
		TimeSpec lasttime_recv;
		boost::posix_time::ptime lasttime_recv_local;


		int32_t inval_lifespan;
		int32_t outval_lifespan;

		int32_t inval_lifespan;
		int32_t outval_lifespan;

		uint32_t endpoint;
		RR_WEAK_PTR<WireBase> parent;
		std::string service_path;
		std::string member_name;

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


		std::list<RR_WEAK_PTR<WireConnectionBaseListener> > listeners;

		RR_WEAK_PTR<RobotRaconteurNode> node;

		MemberDefinition_Direction direction;
		
	};

	
	/**
	 * @brief Wire connection used to transmit "most recent" values
	 * 
	 * Wire connections are used to transmit "most recent" values between connected
	 * wire members. See Wire for more information on wire members.
	 * 
	 * Wire connections are created by clients using the Wire::Connect() or Wire::AsyncConnect()
	 * functions. Services receive incoming wire connection requests through a 
	 * callback function specified using the Wire::SetWireConnectCallback() function. Services
	 * may also use the WireBroadcaster class to automate managing wire connection lifecycles and
	 * sending values to all connected clients, or use WireUnicastReceiver to receive an incoming
	 * value from the most recently connected client.
	 * 
	 * Wire connections are used to transmit "most recent" values between clients and services. Connection
	 * the wire creates a connection pair, one in the client, and one in the service. Each wire connection 
	 * object has an InValue and an OutValue. Setting the OutValue of one will cause the specified value to
	 * be transmitted to the InValue of the peer. See Wire for more information.
	 * 
	 * Values can optionally be specified to have a finite lifespan using SetInValueLifespan() and
	 * SetOutValueLifespan(). Lifespans can be used to prevent using old values that have
	 * not been recently updated.
	 *
	 * This class is instantiated by the Wire class. It should not be instantiated
	 * by the user.
	 * 
	 * @tparam T The value data type
	 */
	template <typename T>
	class WireConnection : public WireConnectionBase
	{
	private:
		boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> WireConnectionClosedCallback;

	public:
		/**
		 * @brief Signal invoked when the InValue is changed
		 * 
		 * Callback function must accept three arguments, receiving the WireConnectionPtr<T> that
		 * received a packet, the new value, and the value's TimeSpec timestamp
		 */
		boost::signals2::signal<void (RR_SHARED_PTR<WireConnection<T> > connection, T value, TimeSpec time)> WireValueChanged;

		/**
		 * @brief Get the currently configured connection closed callback function
		 * 
		 * @return boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> 
		 */

		boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> GetWireConnectionClosedCallback()
		{
			return WireConnectionClosedCallback;
		}

		/**
		 * @brief Set the connection closed callback function
		 * 
		 * Sets a function to invoke when the wire connection has been closed.
		 * 
		 * Callback function must accept one argument, receiving the WireConnectionPtr<T> that
		 * was closed.
		 * 
		 * @param callback The callback function
		 */
		void SetWireConnectionClosedCallback(boost::function<void (RR_SHARED_PTR<WireConnection<T> >)> callback)
		{
			WireConnectionClosedCallback=callback;
		}

		virtual ~WireConnection() {}

		/**
		 * @brief Get the current InValue
		 * 
		 * Gets the current InValue that was transmitted from the peer. Throws
		 * ValueNotSetException if no value has been received, or the most
		 * recent value lifespan has expired.
		 * 
		 * @return T The value
		 */
		virtual T GetInValue()
		{
			return RRPrimUtil<T>::PreUnpack(GetInValueBase());
		}

		/**
		 * @brief Get the current OutValue
		 * 
		 * Gets the current OutValue that was transmitted to the peer. Throws
		 * ValueNotSetException if no value has been received, or the most
		 * recent value lifespan has expired.
		 * 
		 * @return T The value
		 */
		virtual T GetOutValue()
		{
			return RRPrimUtil<T>::PreUnpack(GetOutValueBase());
		}

		/**
		 * @brief Set the OutValue and transmit to the peer connection
		 * 
		 * Sets the OutValue for the wire connection. The specified value will be
		 * transmitted to the peer, and will become the peers InValue. The transmission
		 * is unreliable, meaning that values may be dropped if newer values arrive.
		 * 
		 * @param value The new out value
		 */
		virtual void SetOutValue(typename boost::call_traits<T>::param_type value)
		{
			SetOutValueBase(RRPrimUtil<T>::PrePack(value));
		}

		/**
		 * @brief Try getting the InValue, returning true on success or false on failure
		 * 
		 * Get the current InValue and InValue timestamp. Return true or false on
		 * success or failure instead of throwing exception.
		 * 
		 * @param value [out] The current InValue
		 * @param time [out] The current InValue timestamp in the senders clock
		 * @return true The InValue was valid
		 * @return false The InValue was invalid. value and time are undefined
		 */
		bool TryGetInValue(T& value, TimeSpec& time)
		{
			RR_INTRUSIVE_PTR<RRValue> o;
			if (!TryGetInValueBase(o, time)) return false;
			value = RRPrimUtil<T>::PreUnpack(o);
			return true;
		}

		/**
		 * @brief Try getting the OutValue, returning true on success or false on failure
		 * 
		 * Get the current OutValue and OutValue timestamp. Return true or false on
		 * success and failure instead of throwing exception.
		 * 
		 * @param value [out] The current OutValue
		 * @param time [out] The current OutValue timestamp in the local clock
		 * @return true The OutValue was valid
		 * @return false The OutValue was invalid. value and time are undefined
		 */
		bool TryGetOutValue(T& value, TimeSpec& time)
		{
			RR_INTRUSIVE_PTR<RRValue> o;
			if (!TryGetOutValueBase(o, time)) return false;
			value = RRPrimUtil<T>::PreUnpack(o);
			return true;
		}

		WireConnection(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint=0, MemberDefinition_Direction direction = MemberDefinition_Direction_both)
			: WireConnectionBase(parent,endpoint,direction) {}

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
			WireConnectionBase::AsyncClose(boost::bind(&WireConnection<T>::AsyncClose1,RR_STATIC_POINTER_CAST<WireConnection<T> >(shared_from_this()),RR_BOOST_PLACEHOLDERS(_1),handler),timeout);
			
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


	/**
	 * @brief Base class for Wire
	 * 
	 */
	class ROBOTRACONTEUR_CORE_API  WireBase : public RR_ENABLE_SHARED_FROM_THIS<WireBase>, private boost::noncopyable
	{
		

	public:

		friend class WireConnectionBase;
		
		virtual ~WireBase() {}

		/**
		 * @brief Get the member name of the wire
		 * 
		 * @return std::string 
		 */
		virtual std::string GetMemberName()=0;

		virtual std::string GetServicePath()=0;

		virtual void WirePacketReceived(RR_INTRUSIVE_PTR<MessageEntry> m, uint32_t e=0)=0;

		virtual void Shutdown()=0;


		

		virtual void AsyncClose(RR_SHARED_PTR<WireConnectionBase> endpoint, bool remote, uint32_t ee, RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)=0;

	protected:

		virtual void SendWirePacket(RR_INTRUSIVE_PTR<RRValue> data, TimeSpec time, uint32_t endpoint)=0;

		bool rawelements;		

		void DispatchPacket (RR_INTRUSIVE_PTR<MessageEntry> me, RR_SHARED_PTR<WireConnectionBase> e);

		RR_INTRUSIVE_PTR<RRValue> UnpackPacket(RR_INTRUSIVE_PTR<MessageEntry> me, TimeSpec& ts);

		RR_INTRUSIVE_PTR<MessageEntry> PackPacket(RR_INTRUSIVE_PTR<RRValue> data, TimeSpec time);

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

		/**
		 * @brief The direction of the wire
		 * 
		 * Wires may be declared *readonly* or *writeonly* in the service definition file. (If neither
		 * is specified, the wire is assumed to be full duplex.) *readonly* wire may only send out values from
		 * service to client. *writeonly* wires may only send out values from client to service.
		 * 
		 * @return MemberDefinition_Direction 
		 */
		MemberDefinition_Direction Direction();	

	};


	/**
	 * @brief `wire` member type interface
	 * 
	 * The Wire class implements the `wire` member type. Wires are declared in service definition files
	 * using the `wire` keyword within object declarations. Wires provide "most recent" value streaming
	 * between clients and services. They work by creating "connection" pairs between the client and service.
	 * The wire streams the current value between the wire connection pairs using packets. Wires 
	 * are unreliable; only the most recent value is of interest, and any older values 
	 * will be dropped. Wire connections have an InValue and an OutValue. Users set the OutValue on the
	 * connection. The new OutValue is transmitted to the peer wire connection, and becomes the peer's
	 * InValue. The peer can then read the InValue. The client and service have their own InValue
	 * and OutValue, meaning that each direction, client to service or service to client, has its own
	 * value.
	 * 
	 * Wire connections are created using the Connect() or AsyncConnect() functions. Services receive
	 * incoming connection requests through a callback function. Thes callback is configured using
	 * the SetWireConnectCallback() function. Services may also use the WireBroadcaster class
	 * or WireUnicastReceiver class to automate managing wire connection lifecycles. WireBroadcaster
	 * is used to send values to all connected clients. WireUnicastReceiver is used to receive the
	 * value from the most recent wire connection. See WireConnection for details on sending
	 * and receiving streaming values.
	 * 
	 * Wire clients may also optionally "peek" and "poke" the wire without forming a streaming
	 * connection. This is useful if the client needs to read the InValue or set the OutValue
	 * instantaniously, but does not need continuous updating. PeekInValue() or 
	 * AsyncPeekInValue() will retrieve the client's current InValue. PokeOutValue() or
	 * AsyncPokeOutValue() will send a new client OutValue to the service.
	 * PeekOutValue() or AsyncPeekOutValue() will retrieve the last client OutValue received by
	 * the service.
	 * 
	 * "Peek" and "poke" operations initiated by the client are received on the service using
	 * callbacks. Use SetPeekInValueCallback(), SetPeekOutValueCallback(),
	 * and SetPokeOutValueCallback() to configure the callbacks to handle these requests.
	 * WireBroadcaster and WireUnicastReceiver configure these callbacks automatically, so 
	 * the user does not need to configure the callbacks when these classes are used.
	 * 
	 * Wires can be declared *readonly* or *writeonly*. If neither is specified, the wire is assumed
	 * to be full duplex. *readonly* pipes may only send values from service to client, ie OutValue
	 * on service side and InValue on client side. *writeonly* pipes may only send values from
	 * client to service, ie OutValue on client side and InValue on service side. Use Direction()
	 * to determine the direction of the wire.
	 * 
	 * Unlike pipes, wire connections are not indexed, so only one connection pair can be
	 * created per client connection.
	 *  
	 * WireBroadcaster or WireUnicastReceiver are typically used to simplify using wires.
	 * See WireBroadcaster and WireUnicastReceiver for more information.
	 * 
	 * This class is instantiated by the node. It should not be instantiated by the user.
	 * 
	 * @tparam T The value data type
	 */
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
		
		/**
		 * @brief Connect the wire
		 * 
		 * Creates a connection between the wire, returning the client connection. Used to create
		 * a "most recent" value streaming connection to the service.
		 * 
		 * Only valid on clients. Will throw InvalidOperationException on the service side.
		 * 
		 * Note: If a streaming connection is not required, use PeekInValue(), PeekOutValue(),
		 * or PokeOutValue() instead of creating a connection.
		 * 
		 * @return RR_SHARED_PTR<WireConnection<T> > The wire connection
		 */
		virtual RR_SHARED_PTR<WireConnection<T> > Connect() = 0;

		/**
		 * @brief Asynchronously connect the wire
		 * 
		 * Same as Connect(), but returns asynchronously
		 * 
		 * Only valid on clients. Will throw InvalidOperationException on the service side.
		 * 
		 * @param handler A handler function to receive the wire connection, or an exception
		 * @param timeout Timeout in milliseconds, or RR_TIMEOUT_INFINITE for no timeout
		 */
		virtual void AsyncConnect(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<WireConnection<T> >, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;

		/**
		 * @brief Asynchronously peek the current InValue
		 * 
		 * Same as PeekInValue(), but returns asynchronously.
		 * 
		 * Only valid on clients. Will throw InvalidOperationException on the service side.
		 * 
		 * @param handler A handler function to receive the InValue and timestamp, or an exception
		 * @param timeout Timeout in milliseconds, or RR_TIMEOUT_INFINITE for no timeout
		 */
		virtual void AsyncPeekInValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;

		/**
		 * @brief Asynchronously peek the current OutValue
		 * 
		 * Same as PeekOutValue(), but returns asynchronously.
		 * 
		 * Only valid on clients. Will throw InvalidOperationException on the service side.
		 * 
		 * @param handler A handler function to receive the OutValue and timestamp, or an exception
		 * @param timeout Timeout in milliseconds, or RR_TIMEOUT_INFINITE for no timeout
		 */
		virtual void AsyncPeekOutValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;

		/**
		 * @brief Asynchronously poke the OutValue
		 * 
		 * Same as PokeOutValue(), but returns asynchronously
		 * 
		 * Only valid on clients. Will throw InvalidOperationException on the service side.
		 * 
		 * @param handler A handler function to invoke on completion, with possible exception
		 * @param value The new OutValue
		 * @param timeout Timeout in milliseconds, or RR_TIMEOUT_INFINITE for no timeout
		 */
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
		friend class detail::WireSubscription_connection;

	public:

		virtual ~WireClientBase() {}

		virtual std::string GetMemberName();

		virtual std::string GetServicePath();

		virtual void WirePacketReceived(RR_INTRUSIVE_PTR<MessageEntry> m, uint32_t e = 0);

		virtual void Shutdown();

		virtual void AsyncClose(RR_SHARED_PTR<WireConnectionBase> endpoint, bool remote, uint32_t ee, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout);

		RR_SHARED_PTR<ServiceStub> GetStub();

	protected:

		virtual void SendWirePacket(RR_INTRUSIVE_PTR<RRValue> packet, TimeSpec time, uint32_t endpoint);

		std::string m_MemberName;
		std::string service_path;
		uint32_t endpoint;

		RR_SHARED_PTR<WireConnectionBase> connection;

		RR_WEAK_PTR<ServiceStub> stub;

		void AsyncConnect_internal(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<WireConnectionBase>, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout);

		void AsyncConnect_internal1(RR_INTRUSIVE_PTR<MessageEntry> ret, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_SHARED_PTR<WireConnectionBase>, RR_SHARED_PTR<RobotRaconteurException>)>& handler);


		WireClientBase(boost::string_ref name, RR_SHARED_PTR<ServiceStub> stub, MemberDefinition_Direction direction);

		virtual RR_SHARED_PTR<WireConnectionBase> CreateNewWireConnection(MemberDefinition_Direction direction) = 0;

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
			AsyncConnect_internal(boost::bind(handler,boost::bind(&WireClient<T>::AsyncConnect_cast,RR_BOOST_PLACEHOLDERS(_1)),RR_BOOST_PLACEHOLDERS(_2)),timeout);
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
			AsyncPeekInValueBase(boost::bind(&WireClient::AsyncPeekValueBaseEnd2, RR_DYNAMIC_POINTER_CAST<WireClient>(shared_from_this()), RR_BOOST_PLACEHOLDERS(_1), RR_BOOST_PLACEHOLDERS(_2), RR_BOOST_PLACEHOLDERS(_3), RR_MOVE(handler)), timeout);
		}
		virtual void AsyncPeekOutValue(RR_MOVE_ARG(boost::function<void(const T&, const TimeSpec&, RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			AsyncPeekOutValueBase(boost::bind(&WireClient::AsyncPeekValueBaseEnd2, RR_DYNAMIC_POINTER_CAST<WireClient>(shared_from_this()), RR_BOOST_PLACEHOLDERS(_1), RR_BOOST_PLACEHOLDERS(_2), RR_BOOST_PLACEHOLDERS(_3), RR_MOVE(handler)), timeout);
		}
		virtual void AsyncPokeOutValue(const T& value, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			AsyncPokeOutValueBase(RRPrimUtil<T>::PrePack(value), RR_MOVE(handler), timeout);
		}
		
	protected:
		virtual RR_SHARED_PTR<WireConnectionBase> CreateNewWireConnection(MemberDefinition_Direction direction)
		{
			return RR_MAKE_SHARED<WireConnection<T> >(RR_STATIC_POINTER_CAST<WireBase>(shared_from_this()), 0, direction);
		}

	};

#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES
	/** @brief Convenience alias for WireConnectionBase shared_ptr */
	using WireConnectionBasePtr = RR_SHARED_PTR<WireConnectionBase>;
	/** @brief Convenience alias for WireConnection shared_ptr */
	template <typename T> using WireConnectionPtr = RR_SHARED_PTR<WireConnection<T> >;
	/** @brief Convenience alias for WireBase shared_ptr */
	using WireBasePtr = RR_SHARED_PTR<WireBase>;
	/** @brief Convenience alias for Wire shared_ptr */
	template <typename T> using WirePtr = RR_SHARED_PTR<Wire<T> >;
#endif

}

#pragma warning(pop)
