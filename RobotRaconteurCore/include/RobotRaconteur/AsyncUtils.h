/** 
 * @file AsyncUtils.h
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

#include <boost/scope_exit.hpp>

#include "RobotRaconteur/Error.h"
#include "RobotRaconteur/DataTypes.h"
#include "RobotRaconteur/ErrorUtil.h"
#include "RobotRaconteur/Timer.h"
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>

#pragma once

namespace RobotRaconteur
{
	class ROBOTRACONTEUR_CORE_API RobotRaconteurNode;
	
	namespace detail
	{
		RR_SHARED_PTR<Timer> async_timeout_wrapper_CreateTimer(RR_SHARED_PTR<RobotRaconteurNode> node, const boost::posix_time::time_duration& period, RR_MOVE_ARG(boost::function<void(const TimerEvent&)>) handler, bool oneshot);

		template<typename T>
		void async_timeout_wrapper_closer(RR_SHARED_PTR<T> d) 
		{
			try
			{
				d->Close();
			}
			catch (std::exception&) {}
		}

		static void async_timeout_wrapper_closer_handler() {}

		template<typename T, typename T2>
		void async_timeout_wrapper_closer(RR_SHARED_PTR<T> d) 
		{
			try
			{
				RR_SHARED_PTR<T2> t2=RR_DYNAMIC_POINTER_CAST<T2>(d);
				if (!t2) return;
				t2->AsyncClose(boost::bind(&async_timeout_wrapper_closer_handler));
			}
			catch (std::exception&) {}
		}

		template<typename T>
		class async_timeout_wrapper : public RR_ENABLE_SHARED_FROM_THIS<async_timeout_wrapper<T> >, private boost::noncopyable
		{
		private:
			boost::function<void (RR_SHARED_PTR<T>,RR_SHARED_PTR<RobotRaconteurException>)> handler_;
			RR_SHARED_PTR<Timer> timeout_timer_;
			bool handled;
			RR_SHARED_PTR<RobotRaconteurException> timeout_exception_;
			boost::function<void(RR_SHARED_PTR<T>)> deleter_;
			RR_WEAK_PTR<RobotRaconteurNode> node;

		public:

			async_timeout_wrapper(RR_SHARED_PTR<RobotRaconteurNode> node, boost::function<void (RR_SHARED_PTR<T>,RR_SHARED_PTR<RobotRaconteurException>)> handler, RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<T>)>) deleter=0)
			{
				handler_=handler;
				
				handled=false;
				deleter_.swap(deleter);
				this->node=node;
				
				

			}

			void start_timer(int32_t timeout,  RR_SHARED_PTR<RobotRaconteurException> timeout_exception=RR_MAKE_SHARED<ConnectionException>("Timeout during operation"));

			void operator() (RR_SHARED_PTR<T> data, RR_SHARED_PTR<RobotRaconteurException> err)
			{
				{
					if (handled)
					{
						if (data && deleter_) deleter_(data);
						return;
					}
					handled=true;
					
					try
					{
					if (timeout_timer_) timeout_timer_->Stop();
					}
					catch (std::exception&) {}
					timeout_timer_.reset();
				}

				handler_(data,err);

			}

			void handle_error(RR_SHARED_PTR<RobotRaconteurException> err)
			{
				{
					if (handled) return;
					handled=true;

					{
						
						try
						{
						if (timeout_timer_) timeout_timer_->Stop();
						}
						catch (std::exception&) {}
						timeout_timer_.reset();
					}
				}

				handler_(RR_SHARED_PTR<T>(),err);
			}

			void handle_error(const boost::system::error_code& err)
			{				
				if (err.value()==boost::system::errc::timed_out) handle_error(timeout_exception_);
				handle_error(RR_MAKE_SHARED<ConnectionException>(err.message()));
			}


			

		private:
			void timeout_handler(const TimerEvent& /*e*/)
			{
				{
					if (handled) return;
					handled=true;
				//	timeout_timer_.reset();
				
					
					timeout_timer_.reset();
				}


				handler_(RR_SHARED_PTR<T>(),timeout_exception_);

			}


			
		};

		template<typename T>
		void async_timeout_wrapper<T>::start_timer(int32_t timeout,  RR_SHARED_PTR<RobotRaconteurException> timeout_exception)
		{
			RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
			if (!n) InvalidOperationException("Node has been released");

			if (handled) return;

			if (timeout!=RR_TIMEOUT_INFINITE)
			{
				timeout_timer_=async_timeout_wrapper_CreateTimer(n,boost::posix_time::milliseconds(timeout),boost::bind(&async_timeout_wrapper<T>::timeout_handler,this->shared_from_this(),RR_BOOST_PLACEHOLDERS(_1)),true);
				timeout_timer_->Start();
				timeout_exception_=timeout_exception;
			}
		}


		/*template<typename Handler>
		class handler_move_wrapper
		{
		public:
			handler_move_wrapper(Handler& handler)
				: handler_(RR_MOVE(handler))
			{}

			
			Handler&& operator()
			{
				return RR_MOVE(handler_);
			}
						
		protected:
			Handler handler_;
		};

		template<typename Handler>
		handler_move_wrapper<Handler> make_handler_move_wrapper(Handler& handler)
		{
			return handler_move_wrapper<Handler>(handler);
		}*/
		

		ROBOTRACONTEUR_CORE_API void InvokeHandler_HandleException(RR_WEAK_PTR<RobotRaconteurNode> node, std::exception& exp);

		ROBOTRACONTEUR_CORE_API void InvokeHandler_DoPost(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void()>& h, bool shutdown_op = false, bool throw_on_released = true);

		ROBOTRACONTEUR_CORE_API void InvokeHandler(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void()>& handler);
		
		template<typename T>
		void InvokeHandler(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T)>& handler, T& value)
		{
			try
			{
				handler(value);
			}
			catch (std::exception& exp)
			{
				InvokeHandler_HandleException(node, exp);
			}
		}

		ROBOTRACONTEUR_CORE_API void InvokeHandler(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler);
		
		ROBOTRACONTEUR_CORE_API void InvokeHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler, RR_SHARED_PTR<RobotRaconteurException> exp);

		ROBOTRACONTEUR_CORE_API void InvokeHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler, std::exception& exp, MessageErrorType default_err = MessageErrorType_UnknownError);
		
		template<typename T>
		void InvokeHandler(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T, RR_SHARED_PTR<RobotRaconteurException>)>& handler, T& value)
		{
			try
			{
				handler(value, RR_SHARED_PTR<RobotRaconteurException>());
			}
			catch (std::exception& exp)
			{
				InvokeHandler_HandleException(node, exp);
			}
		}

		template<typename T>
		void InvokeHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T, RR_SHARED_PTR<RobotRaconteurException>)>& handler, RR_SHARED_PTR<RobotRaconteurException> exp)
		{
			typename boost::initialized<T> default_value;
			try
			{
				handler(default_value, exp);
			}
			catch (std::exception& exp)
			{
				InvokeHandler_HandleException(node, exp);
			}
		}

		template<typename T>
		void InvokeHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T, RR_SHARED_PTR<RobotRaconteurException>)>& handler, std::exception& exp, MessageErrorType default_err = MessageErrorType_UnknownError)
		{
			typename boost::initialized<typename boost::remove_const<typename boost::remove_reference<T>::type>::type> default_value;
			try
			{
				RR_SHARED_PTR<RobotRaconteurException> err = RobotRaconteurExceptionUtil::ExceptionToSharedPtr(exp, default_err);
				handler(default_value, err);
			}
			catch (std::exception& exp)
			{
				InvokeHandler_HandleException(node, exp);
			}
		}


		ROBOTRACONTEUR_CORE_API void PostHandler(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void()>& handler, bool shutdown_op = false, bool throw_on_released = true);
		
		template<typename T>
		void PostHandler(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T)>& handler, T& value, bool shutdown_op = false, bool throw_on_released = true)
		{
			boost::function<void()> h = boost::bind(handler, value);
			InvokeHandler_DoPost(node, h, shutdown_op, throw_on_released);
		}

		ROBOTRACONTEUR_CORE_API void PostHandler(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler, bool shutdown_op = false, bool throw_on_released = true);

		ROBOTRACONTEUR_CORE_API void PostHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler, RR_SHARED_PTR<RobotRaconteurException> exp, bool shutdown_op = false, bool throw_on_released = true);

		ROBOTRACONTEUR_CORE_API void PostHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>& handler, std::exception& exp, MessageErrorType default_err = MessageErrorType_UnknownError, bool shutdown_op = false, bool throw_on_released = true);

		template<typename T>
		void PostHandler(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T, RR_SHARED_PTR<RobotRaconteurException>)>& handler, T& value, bool shutdown_op = false, bool throw_on_released = true)
		{
			boost::function<void()> h=boost::bind(handler,value, RR_SHARED_PTR<RobotRaconteurException>());
			InvokeHandler_DoPost(node, h, shutdown_op, throw_on_released);			
		}

		template<typename T>
		void PostHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T, RR_SHARED_PTR<RobotRaconteurException>)>& handler, RR_SHARED_PTR<RobotRaconteurException> exp, bool shutdown_op = false, bool throw_on_released = true)
		{
			typename boost::initialized<T> default_value;
			boost::function<void()> h = boost::bind(handler, default_value, RR_SHARED_PTR<RobotRaconteurException>());
			InvokeHandler_DoPost(node, h, shutdown_op, throw_on_released);			
		}

		template<typename T>
		void PostHandlerWithException(RR_WEAK_PTR<RobotRaconteurNode> node, typename boost::function<void(T, RR_SHARED_PTR<RobotRaconteurException>)>& handler, std::exception& exp, MessageErrorType default_err = MessageErrorType_UnknownError, bool shutdown_op = false, bool throw_on_released = true)
		{
			typename boost::initialized<T> default_value;
			RR_SHARED_PTR<RobotRaconteurException> err = RobotRaconteurExceptionUtil::ExceptionToSharedPtr(exp, default_err);
			boost::function<void()> h = boost::bind(handler, default_value, err);
			InvokeHandler_DoPost(node, h, shutdown_op, throw_on_released);			
		}

	}
}
