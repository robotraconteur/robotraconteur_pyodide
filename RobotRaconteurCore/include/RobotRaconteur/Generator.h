/** 
 * @file Generator.h
 * 
 * @author Dr. John Wason
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
#include "RobotRaconteur/Endpoint.h"
#include "RobotRaconteur/Client.h"

namespace RobotRaconteur
{
	template <typename Return, typename Param>
	class Generator : private boost::noncopyable
	{
	public:
		
		virtual void AsyncNext(const Param& v, boost::function<void(const Return, RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		virtual ~Generator() {}
	};

	template <typename Return>
	class Generator<Return, void> : private boost::noncopyable
	{
	public:
		
		virtual void AsyncNext(boost::function<void(const Return, RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual ~Generator() {}
	};

	template <typename Param>
	class Generator<void, Param> : private boost::noncopyable
	{
	public:
		
		virtual void AsyncNext(const Param& v, boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE) = 0;
		virtual ~Generator() {}
	};

	class ROBOTRACONTEUR_CORE_API GeneratorClientBase
	{
	protected:

		std::string name;
		int32_t id;
		RR_WEAK_PTR<ServiceStub> stub;
		RR_WEAK_PTR<RobotRaconteurNode> node;
		uint32_t endpoint;
		std::string service_path;

		GeneratorClientBase(boost::string_ref name, int32_t id, RR_SHARED_PTR<ServiceStub> stub);

		virtual void AsyncNextBase(RR_INTRUSIVE_PTR<MessageElement> v, boost::function<void(RR_INTRUSIVE_PTR<MessageElement>, RR_SHARED_PTR<RobotRaconteurException>, RR_SHARED_PTR<RobotRaconteurNode>)> handler, int32_t timeout);
		
		static void AsyncNextBase1(RR_INTRUSIVE_PTR<MessageEntry> ret, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_INTRUSIVE_PTR<MessageElement>, RR_SHARED_PTR<RobotRaconteurException>, RR_SHARED_PTR<RobotRaconteurNode>)> handler, RR_WEAK_PTR<RobotRaconteurNode> node);

	public:
		RR_SHARED_PTR <ServiceStub> GetStub();	
		
		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE);
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE);
		std::string GetMemberName();

		virtual ~GeneratorClientBase() {}
	};

	namespace detail
	{
		template <typename Return>
		static void GeneratorClient_AsyncNext1(RR_INTRUSIVE_PTR<MessageElement> v2, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<RobotRaconteurNode> node, boost::function<void(Return, RR_SHARED_PTR<RobotRaconteurException> err)> handler)
		{
			if (err)
			{				
				detail::InvokeHandlerWithException(node, handler, err);
				return;
			}
			Return ret;
			try
			{
				ret = RRPrimUtil<Return>::PreUnpack(node->UnpackAnyType<typename RRPrimUtil<Return>::BoxedType>(v2));
			}
			catch (std::exception& e)
			{
				detail::InvokeHandlerWithException(node, handler, e);
				return;
			}
			detail::InvokeHandler<Return>(node, handler, ret);
		}

		ROBOTRACONTEUR_CORE_API void GeneratorClient_AsyncNext2(RR_INTRUSIVE_PTR<MessageElement> v2, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler);
	}

	template <typename Return, typename Param>
	class GeneratorClient : public Generator<Return,Param>, public GeneratorClientBase
	{
	public:
		GeneratorClient(boost::string_ref name, int32_t id, RR_SHARED_PTR<ServiceStub> stub)
			: GeneratorClientBase(name, id, stub)
		{
		}

		virtual void AsyncNext(const Param& v, boost::function<void(Return, RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			RR_INTRUSIVE_PTR<MessageElement> v1 = CreateMessageElement("",GetStub()->RRGetNode()->template PackAnyType<typename RRPrimUtil<Param>::BoxedType>(RRPrimUtil<Param>::PrePack(v)));
			AsyncNextBase(v1, boost::bind<void>(&detail::GeneratorClient_AsyncNext1<Return>, _1, _2, _3, handler),timeout);
		}		

		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			GeneratorClientBase::AsyncAbort(handler, timeout);
		}
		
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			GeneratorClientBase::AsyncAbort(handler, timeout);
		}
	};
	
	template <typename Return>
	class GeneratorClient<Return,void> : public Generator<Return, void>, public GeneratorClientBase
	{
	public:
		GeneratorClient(boost::string_ref name, int32_t id, RR_SHARED_PTR<ServiceStub> stub)
			: GeneratorClientBase(name, id, stub)
		{
		}
		
		virtual void AsyncNext(boost::function<void(Return, RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{			
			AsyncNextBase(RR_INTRUSIVE_PTR<MessageElement>(), boost::bind<void>(&detail::GeneratorClient_AsyncNext1<Return>, _1, _2, _3, handler), timeout);
		}		
		
		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			GeneratorClientBase::AsyncAbort(handler, timeout);
		}
		
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			GeneratorClientBase::AsyncAbort(handler, timeout);
		}
	};

	template <typename Param>
	class GeneratorClient<void,Param> : public Generator<void, Param>, public GeneratorClientBase
	{
	public:
		GeneratorClient(boost::string_ref name, int32_t id, RR_SHARED_PTR<ServiceStub> stub)
			: GeneratorClientBase(name, id, stub)
		{
		}
		
		virtual void AsyncNext(const Param& v, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			RR_INTRUSIVE_PTR<MessageElement> v1 = CreateMessageElement("", GetStub()->RRGetNode()->template PackAnyType<typename RRPrimUtil<Param>::BoxedType>(RRPrimUtil<Param>::PrePack(v)));
			AsyncNextBase(v1, boost::bind<void>(&detail::GeneratorClient_AsyncNext2, _1, _2, _3, handler), timeout);
		}
		
		virtual void AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			GeneratorClientBase::AsyncAbort(handler, timeout);
		}
		
		virtual void AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout = RR_TIMEOUT_INFINITE)
		{
			GeneratorClientBase::AsyncAbort(handler, timeout);
		}
	};
	
#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES
	template <typename Return, typename Param> using GeneratorPtr = RR_SHARED_PTR<Generator<Return, Param> >;
	template <typename Return, typename Param> using GeneratorConstPtr = RR_SHARED_PTR<const Generator<Return, Param> >;	
#endif

}
