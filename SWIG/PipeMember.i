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

%shared_ptr(RobotRaconteur::WrappedPipeEndpoint)
%shared_ptr(RobotRaconteur::WrappedPipeClient)


%feature("director") RobotRaconteur::WrappedPipeEndpointDirector;
%feature("director") RobotRaconteur::AsyncPipeEndpointReturnDirector;

namespace RobotRaconteur
{
class WrappedPipeEndpointDirector
{
public:
	virtual ~WrappedPipeEndpointDirector() {}
	virtual void PipeEndpointClosedCallback();
	virtual void PacketReceivedEvent();
	virtual void PacketAckReceivedEvent(uint32_t packetnum);

};

class AsyncPipeEndpointReturnDirector
{
public:
	virtual ~AsyncPipeEndpointReturnDirector();
	virtual void handler(boost::shared_ptr<RobotRaconteur::WrappedPipeEndpoint> ep, HandlerErrorInfo& error);
};

class WrappedTryReceivePacketWaitResult
{
public:
    bool res;
	boost::intrusive_ptr<MessageElement> packet;
};

%nodefaultctor WrappedPipeEndpoint;
class WrappedPipeEndpoint
{

public:

	virtual boost::intrusive_ptr<RobotRaconteur::MessageElement> ReceivePacket();
	virtual boost::intrusive_ptr<RobotRaconteur::MessageElement> PeekNextPacket();

	WrappedTryReceivePacketWaitResult TryReceivePacketWait(int32_t timeout = RR_TIMEOUT_INFINITE, bool peek = false);
	
	virtual int32_t GetIndex();
	virtual uint32_t GetEndpoint();
	size_t Available();
	bool GetRequestPacketAck();
	void SetRequestPacketAck(bool v);
	boost::shared_ptr<TypeDefinition> Type;
	//WrappedPipeEndpointDirector* RR_Director;
	//WrappedPipeEndpointDirector* GetRRDirector();
	void SetRRDirector(WrappedPipeEndpointDirector* director, int32_t id);
	
	bool IsUnreliable();
	MemberDefinition_Direction Direction();
	
	virtual void AsyncSendPacket(boost::intrusive_ptr<RobotRaconteur::MessageElement> packet, AsyncUInt32ReturnDirector* handler, int32_t id);
	void AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);
	boost::shared_ptr<RobotRaconteur::RobotRaconteurNode> GetNode();

	bool GetIgnoreReceived();
	void SetIgnoreReceived(bool ignore);
};

%nodefaultctor WrappedPipeClient;
class WrappedPipeClient 
{
public:	

	void AsyncConnect(int32_t index, int32_t timeout, AsyncPipeEndpointReturnDirector* handler, int32_t id);
	std::string GetMemberName();
	boost::shared_ptr<RobotRaconteur::TypeDefinition> Type;
	
	boost::shared_ptr<RobotRaconteur::RobotRaconteurNode> GetNode();

	MemberDefinition_Direction Direction();
};

}



