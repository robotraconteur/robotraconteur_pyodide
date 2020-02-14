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

%shared_ptr(RobotRaconteur::WrappedWireConnection)
%shared_ptr(RobotRaconteur::WrappedWireClient)
%shared_ptr(RobotRaconteur::WrappedWireServer)
%shared_ptr(RobotRaconteur::WrappedWireBroadcaster)
%shared_ptr(RobotRaconteur::WrappedWireUnicastReceiver)

%feature("director") RobotRaconteur::WrappedWireConnectionDirector;
%feature("director") RobotRaconteur::AsyncWireConnectionReturnDirector;
%feature("director") RobotRaconteur::AsyncWirePeekReturnDirector;
%feature("director") RobotRaconteur::WrappedWireServerConnectDirector;
%feature("director") RobotRaconteur::WrappedWireServerPeekValueDirector;
%feature("director") RobotRaconteur::WrappedWireServerPokeValueDirector;
%feature("director") RobotRaconteur::WrappedWireBroadcasterPredicateDirector;

namespace RobotRaconteur
{

class TimeSpec;

class WrappedWireConnectionDirector
{
public:
	virtual ~WrappedWireConnectionDirector() {}
	virtual void WireValueChanged(boost::intrusive_ptr<RobotRaconteur::MessageElement> value, const TimeSpec& time);
	virtual void WireConnectionClosedCallback();
};

class AsyncWireConnectionReturnDirector
{
public:
	virtual ~AsyncWireConnectionReturnDirector();
	virtual void handler(boost::shared_ptr<RobotRaconteur::WrappedWireConnection> ep, HandlerErrorInfo& error);
};

class AsyncWirePeekReturnDirector
{
public:
	virtual ~AsyncWirePeekReturnDirector() {}
	virtual void handler(boost::intrusive_ptr<RobotRaconteur::MessageElement> value, const TimeSpec& ts, HandlerErrorInfo& error) {};
};

class TryGetValueResult
{
public:
	bool res;
	boost::intrusive_ptr<RobotRaconteur::MessageElement> value;
	TimeSpec ts;
};

%nodefaultctor WrappedWireConnection;
class WrappedWireConnection
{
public:

	virtual boost::intrusive_ptr<MessageElement> GetInValue();
	virtual boost::intrusive_ptr<MessageElement> GetOutValue();
	virtual void SetOutValue(boost::intrusive_ptr<MessageElement> value);
	
	//WrappedWireConnectionDirector* RR_Director;
	
	//WrappedWireConnectionDirector* GetRRDirector();
	
	void SetRRDirector(WrappedWireConnectionDirector* director, int32_t id);
	
	boost::shared_ptr<TypeDefinition> Type;

	virtual uint32_t GetEndpoint();

	TimeSpec GetLastValueReceivedTime();
	
	TimeSpec GetLastValueSentTime();
	
	bool GetInValueValid();

	bool GetOutValueValid();
	
	TryGetValueResult TryGetInValue();
	TryGetValueResult TryGetOutValue();

	void AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);
	
	boost::shared_ptr<RobotRaconteur::RobotRaconteurNode> GetNode();

	bool GetIgnoreInValue();
	void SetIgnoreInValue(bool ignore);

    MemberDefinition_Direction Direction();

	
};

%nodefaultctor WrappedWireClient;
class WrappedWireClient
{
public:

	void AsyncConnect(int32_t timeout, AsyncWireConnectionReturnDirector* handler, int32_t id);
	virtual std::string GetMemberName();

	void AsyncPeekInValue(int32_t timeout, AsyncWirePeekReturnDirector* handler, int32_t id);
	void AsyncPeekOutValue(int32_t timeout, AsyncWirePeekReturnDirector* handler, int32_t id);
	void AsyncPokeOutValue(const boost::intrusive_ptr<RobotRaconteur::MessageElement>& value, int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);
		
	
	boost::shared_ptr<RobotRaconteur::TypeDefinition> Type;
	
	boost::shared_ptr<RobotRaconteur::RobotRaconteurNode> GetNode();

	MemberDefinition_Direction Direction();
};

}
