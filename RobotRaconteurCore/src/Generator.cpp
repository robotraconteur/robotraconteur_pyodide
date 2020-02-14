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

#include "RobotRaconteur/Generator.h"
#include "RobotRaconteur/RobotRaconteurNode.h"
#include "RobotRaconteur/Client.h"
#include "RobotRaconteur/DataTypes.h"

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>


namespace RobotRaconteur
{
	GeneratorClientBase::GeneratorClientBase(boost::string_ref name, int32_t id, RR_SHARED_PTR<ServiceStub> stub)
	{
		this->name = RR_MOVE(name.to_string());
		this->id = id;
		this->stub = stub;
	}

	RR_SHARED_PTR<ServiceStub> GeneratorClientBase::GetStub()
	{
		RR_SHARED_PTR<ServiceStub> out = stub.lock();
		if (!out) throw InvalidOperationException("Generator has been closed");
		return out;
	}
	
	void GeneratorClientBase::AsyncAbort(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout)
	{
		RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_GeneratorNextReq, GetMemberName());
		AbortOperationException err("Generator abort requested");
		RobotRaconteurExceptionUtil::ExceptionToMessageEntry(err, m);
		m->AddElement("index", ScalarToRRArray(id));
		GetStub()->AsyncProcessRequest(m, boost::bind(handler, _2), timeout);
	}
	
	void GeneratorClientBase::AsyncClose(boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler, int32_t timeout)
	{
		RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_GeneratorNextReq, GetMemberName());
		StopIterationException err("");
		RobotRaconteurExceptionUtil::ExceptionToMessageEntry(err, m);
		m->AddElement("index", ScalarToRRArray(id));
		GetStub()->AsyncProcessRequest(m, boost::bind(handler, _2), timeout);
	}

	std::string GeneratorClientBase::GetMemberName()
	{
		return name;
	}
	
	void GeneratorClientBase::AsyncNextBase(RR_INTRUSIVE_PTR<MessageElement> v, boost::function<void(RR_INTRUSIVE_PTR<MessageElement> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<RobotRaconteurNode>)> handler, int32_t timeout)
	{
		RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_GeneratorNextReq, GetMemberName());
		m->AddElement("index", ScalarToRRArray(id));
		if (v)
		{
			v->ElementName = "parameter";
			m->elements.push_back(v);
		}
		RR_WEAK_PTR<RobotRaconteurNode> node = GetStub()->RRGetNode();
		GetStub()->AsyncProcessRequest(m, boost::bind(&GeneratorClientBase::AsyncNextBase1,_1,_2,handler,node));
		
	}

	void GeneratorClientBase::AsyncNextBase1(RR_INTRUSIVE_PTR<MessageEntry> ret, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_INTRUSIVE_PTR<MessageElement>, RR_SHARED_PTR<RobotRaconteurException>, RR_SHARED_PTR<RobotRaconteurNode>)> handler, RR_WEAK_PTR<RobotRaconteurNode> node)
	{
		RR_SHARED_PTR<RobotRaconteurNode> node1 = node.lock();

		if (!node1)
		{
			handler(RR_INTRUSIVE_PTR<MessageElement>(), RR_MAKE_SHARED<InvalidOperationException>("Node has been released"), node1);
			return;
		}

		RR_INTRUSIVE_PTR<MessageElement> mret;
		if (err)
		{
			handler(mret, err, node1);
			return;
		}

		ret->TryFindElement("return", mret);
		handler(mret,err,node1);
	}

	namespace detail
	{
		void GeneratorClient_AsyncNext2(RR_INTRUSIVE_PTR<MessageElement> v2, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<RobotRaconteurNode> node, boost::function<void(RR_SHARED_PTR<RobotRaconteurException> err)> handler)
		{
			if (err)
			{
				detail::InvokeHandlerWithException(node, handler, err);
				return;
			}
			detail::InvokeHandler(node, handler);
		}
	}
		
}