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
#include "RobotRaconteur/NodeID.h"
#include "RobotRaconteur/Message.h"
#include "RobotRaconteur/ServiceFactory.h"
#include "RobotRaconteur/Transport.h"
#include "RobotRaconteur/Error.h"
#include "RobotRaconteur/AsyncUtils.h"
#include "RobotRaconteur/Timer.h"
#include "RobotRaconteur/Discovery.h"
#include "RobotRaconteur/DataTypesPacking.h"
#include <queue>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <boost/bind/protect.hpp>
#include <boost/random.hpp>
#include <boost/random/random_device.hpp>

#ifdef ROBOTRACONTEUR_WINDOWS

#undef SendMessage

#endif

namespace boost
{
namespace random
{
	class random_device;
}
}

namespace RobotRaconteur
{

	enum RobotRaconteurObjectLockFlags
	{
		RobotRaconteurObjectLockFlags_USER_LOCK = 0,
		RobotRaconteurObjectLockFlags_CLIENT_LOCK
	};
	
	class ROBOTRACONTEUR_CORE_API ServiceInfo2Subscription;
	class ROBOTRACONTEUR_CORE_API ServiceSubscription;
	class ROBOTRACONTEUR_CORE_API ServiceSubscriptionFilter;

	namespace detail
	{
		class Discovery;
	}

	class ROBOTRACONTEUR_CORE_API RobotRaconteurNode : boost::noncopyable, public RR_ENABLE_SHARED_FROM_THIS<RobotRaconteurNode>
	{
	private:
		
		bool is_shutdown;
		boost::signals2::signal<void ()> shutdown_listeners;
				
		static bool is_init;

		bool instance_is_init;	

	public:

		friend class detail::Discovery;
		friend class Transport;
		friend class WallTimer;
		
		void Init();

		RobotRaconteurNode();

		virtual ~RobotRaconteurNode();
		

		static RobotRaconteurNode* s();

		static RR_SHARED_PTR<RobotRaconteurNode> sp();

		RobotRaconteur::NodeID NodeID();

		std::string NodeName();

		void SetNodeID(const RobotRaconteur::NodeID& id);

		void SetNodeName(boost::string_ref name);

		RR_SHARED_PTR<ServiceFactory> GetServiceType(boost::string_ref servicename);
		
		bool IsServiceTypeRegistered(boost::string_ref servicename);

		void RegisterServiceType(RR_SHARED_PTR<ServiceFactory> factory);


		void UnregisterServiceType(boost::string_ref type);


		std::vector<std::string> GetRegisteredServiceTypes();

		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackStructure(RR_INTRUSIVE_PTR<RRStructure> structure);

		RR_INTRUSIVE_PTR<RRStructure> UnpackStructure(RR_INTRUSIVE_PTR<MessageElementNestedElementList> structure);

		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackPodArray(RR_INTRUSIVE_PTR<RRPodBaseArray> structure);

		RR_INTRUSIVE_PTR<RRPodBaseArray> UnpackPodArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> structure);
		
		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackPodMultiDimArray(RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray> structure);

		RR_INTRUSIVE_PTR<RRPodBaseMultiDimArray> UnpackPodMultiDimArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> structure);

		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackNamedArray(RR_INTRUSIVE_PTR<RRNamedBaseArray> structure);

		RR_INTRUSIVE_PTR<RRNamedBaseArray> UnpackNamedArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> structure);

		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackNamedMultiDimArray(RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray> structure);

		RR_INTRUSIVE_PTR<RRNamedBaseMultiDimArray> UnpackNamedMultiDimArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> structure);


		template <typename T>
		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackMultiDimArray(RR_INTRUSIVE_PTR<RRMultiDimArray<T> > arr)
		{
			return detail::packing::PackMultiDimArray<T>(arr);
		}

		template <typename T>
		RR_INTRUSIVE_PTR<RRMultiDimArray<T> > UnpackMultiDimArray(RR_INTRUSIVE_PTR<MessageElementNestedElementList> ar)
		{
			return detail::packing::UnpackMultiDimArray<T>(ar);
		}

	
		RR_INTRUSIVE_PTR<MessageElementData> PackVarType(RR_INTRUSIVE_PTR<RRValue> vardata);

		RR_INTRUSIVE_PTR<RRValue> UnpackVarType(RR_INTRUSIVE_PTR<MessageElement> mvardata);
	
	public:

		template<typename K, typename T, typename U>
		RR_INTRUSIVE_PTR<MessageElementNestedElementList> PackMapType(const U& set)
		{
			return detail::packing::PackMapType<K,T,U>(set,this);
		}

		template<typename K, typename T>
		RR_INTRUSIVE_PTR<RRMap<K,T> > UnpackMapType(const RR_INTRUSIVE_PTR<MessageElementNestedElementList>& mset)
		{
			return detail::packing::UnpackMapType<K,T>(mset,this);
		}	


		template<typename T, typename U>
		RR_INTRUSIVE_PTR<MessageElementNestedElementList > PackListType(U& set)
		{
			return detail::packing::PackListType<T, U>(set,this);
		}

		template<typename T>
		RR_INTRUSIVE_PTR<RRList<T> > UnpackListType(const RR_INTRUSIVE_PTR<MessageElementNestedElementList >& mset)
		{
			return detail::packing::UnpackListType<T>(mset, this);
		}

	public:
		
		template<typename T, typename U> RR_INTRUSIVE_PTR<MessageElementData> PackAnyType(const RR_INTRUSIVE_PTR<U>& data)
		{
			return detail::packing::PackAnyType<T,U>(data,this);
		}

		template<typename T> T UnpackAnyType(const RR_INTRUSIVE_PTR<MessageElement>& mdata)
		{
			return detail::packing::UnpackAnyType<T>(mdata,this);
		}
		

		virtual void Shutdown();

		typedef boost::signals2::connection shutdown_listener_connection;
		template<typename Handler>
		shutdown_listener_connection AddShutdownListener(BOOST_ASIO_MOVE_ARG(Handler) h)
		{
			return shutdown_listeners.connect(h);
		}
				
		uint32_t RegisterTransport(RR_SHARED_PTR<Transport> transport);

		void AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& callback);

		void MessageReceived(RR_INTRUSIVE_PTR<Message> m);
	protected:

		void TransportConnectionClosed(uint32_t endpoint);

	private:
		uint32_t RequestTimeout;

		uint32_t TransportInactivityTimeout;

		uint32_t EndpointInactivityTimeout;
		
		uint32_t MemoryMaxTransferSize;
				
	public:

		uint32_t GetRequestTimeout();
		void SetRequestTimeout(uint32_t timeout);

		uint32_t GetTransportInactivityTimeout();
		void SetTransportInactivityTimeout(uint32_t timeout);

		uint32_t GetEndpointInactivityTimeout();
		void SetEndpointInactivityTimeout(uint32_t timeout);

		uint32_t GetMemoryMaxTransferSize();
		void SetMemoryMaxTransferSize(uint32_t size);

	protected:
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Endpoint> > endpoints;
		std::map<uint32_t,boost::posix_time::ptime> recent_endpoints;
		
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<Transport> > transports;
	
		RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> dynamic_factory;
	public:

		const RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> GetDynamicServiceFactory() ;


		void SetDynamicServiceFactory(RR_SHARED_PTR<RobotRaconteur::DynamicServiceFactory> f);

		RR_INTRUSIVE_PTR<Message> GenerateErrorReturnMessage(RR_INTRUSIVE_PTR<Message> m, MessageErrorType err, boost::string_ref errname, boost::string_ref errdesc);

		public:
		
		void AsyncConnectService(boost::string_ref url, boost::string_ref username, RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > credentials, boost::function<void (RR_SHARED_PTR<ClientContext>,ClientServiceListenerEventType,RR_SHARED_PTR<void>)> listener, boost::string_ref objecttype, boost::function<void(RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);

		void AsyncConnectService(const std::vector<std::string> &url, boost::string_ref username, RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > credentials, boost::function<void (RR_SHARED_PTR<ClientContext>,ClientServiceListenerEventType,RR_SHARED_PTR<void>)> listener, boost::string_ref objecttype, boost::function<void(RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);
		
		void AsyncDisconnectService(RR_SHARED_PTR<RRObject> obj, boost::function<void()> handler);

		std::map<std::string, RR_INTRUSIVE_PTR<RRValue> > GetServiceAttributes(RR_SHARED_PTR<RRObject> obj);

		uint32_t RegisterEndpoint(RR_SHARED_PTR<Endpoint> e);

		void DeleteEndpoint(RR_SHARED_PTR<Endpoint> e);

		void CheckConnection(uint32_t endpoint);



	private:

		
		uint32_t transport_count;

		static RobotRaconteurNode m_s;
		static RR_SHARED_PTR<RobotRaconteurNode> m_sp;

		RobotRaconteur::NodeID m_NodeID;
		std::string m_NodeName;

		bool NodeID_set;
		bool NodeName_set;

		RR_UNORDERED_MAP<std::string,RR_SHARED_PTR<ServiceFactory> > service_factories;
		
		RR_SHARED_PTR<detail::Discovery> m_Discovery;
		boost::signals2::signal<void(const NodeDiscoveryInfo&, const std::vector<ServiceInfo2>& )> discovery_updated_listeners;

		boost::signals2::signal<void(const NodeDiscoveryInfo&)> discovery_lost_listeners;
				
	public:
		std::vector<NodeDiscoveryInfo> GetDetectedNodes();

		typedef boost::signals2::connection node_updated_listener_connection;
		template<typename Handler>
		node_updated_listener_connection AddNodeServicesDetectedListener(BOOST_ASIO_MOVE_ARG(Handler) h)
		{
			if (is_shutdown)
			{
				throw InvalidOperationException("Node has been shut down");
			}

			return discovery_updated_listeners.connect(h);
		}

		typedef boost::signals2::connection node_lost_listener_connection;
		template<typename Handler>
		node_lost_listener_connection AddNodeDetectionLostListener(BOOST_ASIO_MOVE_ARG(Handler) h)
		{
			if (is_shutdown)
			{
				throw InvalidOperationException("Node has been shut down");
			}
			return discovery_lost_listeners.connect(h);
		}

		void NodeAnnouncePacketReceived(boost::string_ref packet);

		void NodeDetected(const NodeDiscoveryInfo& info);
		
		void AsyncUpdateDetectedNodes(const std::vector<std::string>& schemes, boost::function<void()> handler, int32_t timeout = 5000);

		uint32_t GetNodeDiscoveryMaxCacheCount();
		void SetNodeDiscoveryMaxCacheCount(uint32_t count);

		RR_SHARED_PTR<ServiceInfo2Subscription> SubscribeServiceInfo2(const std::vector<std::string>& service_types, RR_SHARED_PTR<ServiceSubscriptionFilter> filter = RR_SHARED_PTR<ServiceSubscriptionFilter>());

		RR_SHARED_PTR<ServiceSubscription> SubscribeService(const std::vector<std::string>& service_types, RR_SHARED_PTR<ServiceSubscriptionFilter> filter = RR_SHARED_PTR<ServiceSubscriptionFilter>());

	protected:
		void CleanDiscoveredNodes();

		void FireNodeDetected(RR_SHARED_PTR<NodeDiscoveryInfo> node, RR_SHARED_PTR<std::vector<ServiceInfo2> > services);

		void FireNodeLost(RR_SHARED_PTR<NodeDiscoveryInfo> node);

	public:
		static std::string SelectRemoteNodeURL(const std::vector<std::string>& urls);
		
		void AsyncFindServiceByType(boost::string_ref servicetype, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<ServiceInfo2> >) > handler, int32_t timeout=5000);
		
		void AsyncFindNodeByID(const RobotRaconteur::NodeID& id, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<NodeInfo2> >) > handler, int32_t timeout=5000);

		void AsyncFindNodeByName(boost::string_ref name, const std::vector<std::string>& transportschemes, boost::function< void(RR_SHARED_PTR<std::vector<NodeInfo2> >) > handler, int32_t timeout=5000);

	public:
		void AsyncRequestObjectLock(RR_SHARED_PTR<RRObject> obj, RobotRaconteurObjectLockFlags flags, boost::function<void(RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);

		void AsyncReleaseObjectLock(RR_SHARED_PTR<RRObject> obj, boost::function<void(RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);

	private:
				
		RR_SHARED_PTR<Timer> PeriodicCleanupTask_timer;

		static void StartPeriodicCleanupTask(RR_SHARED_PTR<RobotRaconteurNode> node);

		void PeriodicCleanupTask(const TimerEvent& err);
		bool PeriodicCleanupTask_timerstarted;

		std::list<RR_SHARED_PTR<IPeriodicCleanupTask> > cleanupobjs;

	public:
		void AddPeriodicCleanupTask(RR_SHARED_PTR<IPeriodicCleanupTask> task);

		void RemovePeriodicCleanupTask(RR_SHARED_PTR<IPeriodicCleanupTask> task);

		

	public:
		//Utility functions to help retrieve objrefs with a specific type
		
		void AsyncFindObjRefTyped(RR_SHARED_PTR<RRObject> obj, boost::string_ref objref, boost::string_ref objecttype, boost::function<void (RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);

		void AsyncFindObjRefTyped(RR_SHARED_PTR<RRObject> obj, boost::string_ref objref, boost::string_ref index, boost::string_ref objecttype, boost::function<void (RR_SHARED_PTR<RRObject>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);

		
		void AsyncFindObjectType(RR_SHARED_PTR<RRObject> obj, boost::string_ref n, boost::function<void (RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);

		void AsyncFindObjectType(RR_SHARED_PTR<RRObject> obj, boost::string_ref n, boost::string_ref i, boost::function<void (RR_SHARED_PTR<std::string>,RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout=RR_TIMEOUT_INFINITE);


	public:
		
		template<typename HandlerType>
		static bool TryPostToThreadPool(RR_WEAK_PTR<RobotRaconteurNode> node, BOOST_ASIO_MOVE_ARG(HandlerType) h, bool shutdown_op=false)
		{			
			RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
			if(!n) return false;
			n->Post(h);
			return true;
		}
				
		void Post(boost::function<void()> f);

	public:

		std::vector<std::string> GetPulledServiceTypes(RR_SHARED_PTR<RRObject> obj);

		RR_SHARED_PTR<ServiceFactory> GetPulledServiceType(RR_SHARED_PTR<RRObject> obj, boost::string_ref type);

		void SetExceptionHandler(boost::function<void (const std::exception*)> handler);

		boost::function<void (const std::exception*)> GetExceptionHandler();

		void HandleException(const std::exception* exp);

		static bool TryHandleException(RR_WEAK_PTR<RobotRaconteurNode> node, const std::exception* exp);

	protected:

		boost::function<void (const std::exception*) > exception_handler;
		RR_SHARED_PTR<boost::random::random_device> random_generator;

	public:

		virtual boost::posix_time::ptime NowUTC();

	protected:

		RR_WEAK_PTR<ITransportTimeProvider> time_provider;

	public:

		virtual RR_SHARED_PTR<Timer> CreateTimer(const boost::posix_time::time_duration& period, boost::function<void (const TimerEvent&)> handler, bool oneshot=false);
		
		virtual void AsyncSleep(const boost::posix_time::time_duration& duration, boost::function<void()> handler);

		void DownCastAndThrowException(RobotRaconteurException& exp);

		RR_SHARED_PTR<RobotRaconteurException> DownCastException(RR_SHARED_PTR<RobotRaconteurException> exp);

		virtual std::string GetServicePath(RR_SHARED_PTR<RRObject> obj);

		bool IsEndpointLargeTransferAuthorized(uint32_t endpoint);

		std::string GetRobotRaconteurVersion();

		template<typename T>
		T GetRandomInt(T min, T max)
		{
			boost::random::uniform_int_distribution<T> d(min, max);
			return d(*random_generator);
		}

		template<typename T>
		std::vector<T> GetRandomInts(size_t count, T min, T max)
		{
			std::vector<T> o;
			o.resize(count);
			for (size_t i = 0; i < count; i++)
			{
				boost::random::uniform_int_distribution<T> d(min, max);
				o[i]=d(*random_generator);
			}
			return o;
		}

		std::string GetRandomString(size_t count);

	};

#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES
	using RobotRaconteurNodePtr = RR_SHARED_PTR<RobotRaconteurNode>;
#endif

}
