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
#include "RobotRaconteur/Endpoint.h"

namespace RobotRaconteur
{
	template<typename T>
	class Callback : private boost::noncopyable
	{

	protected:
		std::string m_MemberName;

	public:
		Callback(const std::string& name)
		{
			m_MemberName = name;
		}

		virtual ~Callback() {}

		virtual T GetFunction() = 0;
		virtual void SetFunction(T value) = 0;

		virtual T GetClientFunction(RR_SHARED_PTR<Endpoint> e) = 0;

		virtual T GetClientFunction(uint32_t e) = 0;

		virtual std::string GetMemberName()
		{
			return m_MemberName;
		}

		virtual void Shutdown()
		{
			
		}


	};

	template<typename T>
	class CallbackClient : public Callback<T>
	{
	public:
		CallbackClient(const std::string& name) : Callback<T>(name)
		{
			InitializeInstanceFields();
		}

		virtual ~CallbackClient() {}


	private:
		T function;
	public:
		virtual T GetFunction() 
		{
			if (!function) throw InvalidOperationException("Callback function not set");
			return function;
		}
		virtual void SetFunction(T value)
		{
			function = value;
		}

		virtual T GetClientFunction(RR_SHARED_PTR<Endpoint> e)
		{
			throw InvalidOperationException("Invalid for client side of callback");
		}

		virtual T GetClientFunction(uint32_t e)
		{
			throw InvalidOperationException("Invalid for client side of callback");
		}

		virtual void Shutdown()
		{
			function.clear();
		}



	private:
		void InitializeInstanceFields()
		{
			function = T();
		}
	};	

#ifndef BOOST_NO_CXX11_TEMPLATE_ALIASES
	template<typename T> using CallbackPtr = RR_SHARED_PTR<Callback<T> >;
	template<typename T> using CallbackConstPtr = RR_SHARED_PTR<const Callback<T> >;
#endif
}
