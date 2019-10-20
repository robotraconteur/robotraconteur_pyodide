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

#include "PipeMember_private.h"
#include "RobotRaconteur/RobotRaconteurNode.h"
#include "RobotRaconteur/Client.h"
#include "RobotRaconteur/DataTypes.h"

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

#define RR_PIPE_ENDPOINT_LISTENER_ITER(command) \
		try \
		{ \
		for (std::list<RR_WEAK_PTR<PipeEndpointBaseListener> >::iterator e = listeners.begin(); e != listeners.end();) \
		{ \
			RR_SHARED_PTR<PipeEndpointBaseListener> p1 = e->lock(); \
			if (!p1) \
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
	void PipeMember_empty_handler(RR_SHARED_PTR<RobotRaconteurException>) {}

PipeEndpointBase::PipeEndpointBase(RR_SHARED_PTR<PipeBase> parent, int32_t index, uint32_t endpoint, bool unreliable, MemberDefinition_Direction direction, bool message3)
{
	send_packet_number=0;
	recv_packet_number=0;

	this->index=index;
	this->parent=parent;
	this->endpoint=endpoint;
	this->unreliable=unreliable;
	this->direction = direction;

	RequestPacketAck=false;
	ignore_incoming_packets = false;
	this->message3 = message3;
	closed = false;
	this->node = parent->GetNode();

}

RR_SHARED_PTR<RobotRaconteurNode> PipeEndpointBase::GetNode()
{
	return GetParent()->GetNode();
}

int32_t PipeEndpointBase::GetIndex()
{
	return index;
}

uint32_t PipeEndpointBase::GetEndpoint()
{
	return endpoint;
}

size_t PipeEndpointBase::Available()
{
	return recv_packets.size();
}

void PipeEndpointBase::AsyncClose(RR_MOVE_ARG(boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
{
	{
		//recv_packets.clear();
		closed = true;		
	}

	{
	GetParent()->AsyncClose(shared_from_this(),false,endpoint,RR_MOVE(handler),timeout);
	}
	
}

bool PipeEndpointBase::IsUnreliable()
{
	return unreliable;
}

MemberDefinition_Direction PipeEndpointBase::Direction()
{
	return direction;
}

void PipeEndpointBase_RemoteClose_emptyhandler(RR_SHARED_PTR<RobotRaconteurException> err) {}

void PipeEndpointBase::RemoteClose()
{
	{
		closed = true;		
	}

	RR_PIPE_ENDPOINT_LISTENER_ITER(p1->PipeEndpointClosed(shared_from_this()));

	try
	{
	fire_PipeEndpointClosedCallback();
	}
	catch (std::exception&) {};
		try
		{
			//if (parent.expired()) return;
			//boost::mutex::scoped_lock lock2 (recvlock);
			GetParent()->AsyncClose(shared_from_this(),true,endpoint,&PipeEndpointBase_RemoteClose_emptyhandler,1000);
		}
		catch (std::exception&)
		{
		}
}

void PipeEndpointBase::AsyncSendPacketBase(RR_INTRUSIVE_PTR<RRValue> packet, RR_MOVE_ARG(boost::function<void(uint32_t,RR_SHARED_PTR<RobotRaconteurException>)>) handler)
{ 
	if (direction == MemberDefinition_Direction_readonly)
	{
		throw ReadOnlyMemberException("Read only pipe");
	}
	
	{
		send_packet_number = (send_packet_number < UINT_MAX) ? send_packet_number + 1 : 0;

		GetParent()->AsyncSendPipePacket(packet, index, send_packet_number, RequestPacketAck, endpoint,unreliable,message3,RR_MOVE(handler));
		
	}


}

RR_INTRUSIVE_PTR<RRValue> PipeEndpointBase::ReceivePacketBase()
{	
	RR_INTRUSIVE_PTR<RRValue> o;
	if (!TryReceivePacketBase(o))
	{
		throw InvalidOperationException("Pipe endpoint receive queue is empty");
	}
	return o;	
}

RR_INTRUSIVE_PTR<RRValue> PipeEndpointBase::PeekPacketBase()
{
	RR_INTRUSIVE_PTR<RRValue> o;
	if (!TryReceivePacketBase(o, true))
	{
		throw InvalidOperationException("Pipe endpoint receive queue is empty");
	}
	return o;
}


bool PipeEndpointBase::TryReceivePacketBase(RR_INTRUSIVE_PTR<RRValue>& packet, bool peek)
{
	if (direction == MemberDefinition_Direction_writeonly)
	{
		throw WriteOnlyMemberException("Write only pipe");
	}
	if (recv_packets.empty())
	{
		return false;
	}

	packet = recv_packets.front();
	if (!peek)
	{
		recv_packets.pop_front();
	}
	return true;

}

static bool PipeEndpointBase_PipePacketReceived_recvpacket(std::deque<RR_INTRUSIVE_PTR<RRValue> >& q, RR_INTRUSIVE_PTR<RRValue>& packet)
{
	std::deque<RR_INTRUSIVE_PTR<RRValue> >::iterator e = q.begin();
	if (e == q.end()) return false;
	packet = *e;
	q.pop_front();
	return true;
}

void PipeEndpointBase::PipePacketReceived(RR_INTRUSIVE_PTR<RRValue> packet, uint32_t packetnum)
{	
	if (direction == MemberDefinition_Direction_writeonly)
	{
		return;
	}

	if (unreliable)
	{
		if (ignore_incoming_packets) return;
		recv_packets.push_back(packet);
						
		RR_PIPE_ENDPOINT_LISTENER_ITER(p1->PipePacketReceived(shared_from_this(), boost::bind(&PipeEndpointBase_PipePacketReceived_recvpacket, boost::ref(recv_packets), _1)));
		
		if (!recv_packets.empty())
		{			

			{
				try
				{
					fire_PacketReceivedEvent();

				}
				catch (std::exception&)
				{
				}

			}
		}
		return;
	}
	else
	{
		{
			if (ignore_incoming_packets) return;
			if (packetnum == increment_packet_number(recv_packet_number))
			{
				recv_packets.push_back(packet);
				recv_packet_number = increment_packet_number(recv_packet_number);
				if (out_of_order_packets.size() > 0)
				{
					while (out_of_order_packets.find(increment_packet_number(recv_packet_number))!= out_of_order_packets.end())
					{
						recv_packet_number = increment_packet_number(recv_packet_number);
						RR_INTRUSIVE_PTR<RRValue> opacket = out_of_order_packets[recv_packet_number];
						recv_packets.push_back(opacket);
						out_of_order_packets.erase(recv_packet_number);

					}

				}

				RR_PIPE_ENDPOINT_LISTENER_ITER(p1->PipePacketReceived(shared_from_this(), boost::bind(&PipeEndpointBase_PipePacketReceived_recvpacket, boost::ref(recv_packets), _1)));

				if (!recv_packets.empty())
				{
					//if (PacketReceivedEvent != 0)
					{
						try
						{							
							fire_PacketReceivedEvent();
						}
						catch (std::exception&)
						{
						}

					}
				}
			}
			else
			{
				out_of_order_packets.insert(std::make_pair(packetnum, packet));
			}
		}
	}
}

void PipeEndpointBase::PipePacketAckReceived(uint32_t packetnum)
{
	RR_PIPE_ENDPOINT_LISTENER_ITER(p1->PipePacketAckReceived(shared_from_this(), packetnum));

	fire_PacketAckReceivedEvent(packetnum);
}

uint32_t PipeEndpointBase::increment_packet_number(uint32_t packetnum)
{
	return (packetnum < std::numeric_limits<uint32_t>::max()) ? packetnum + 1 : 0;

}

RR_SHARED_PTR<PipeBase> PipeEndpointBase::GetParent()
{
	RR_SHARED_PTR<PipeBase> out=parent.lock();
	if (!out) throw InvalidOperationException("Pipe endpoint has been closed");
	return out;
}

bool PipeEndpointBase::GetRequestPacketAck()
{
	return RequestPacketAck;
}

void PipeEndpointBase::SetRequestPacketAck(bool ack)
{
	RequestPacketAck=ack;
}

bool PipeEndpointBase::GetIgnoreReceived()
{
	return ignore_incoming_packets;
}
void PipeEndpointBase::SetIgnoreReceived(bool ignore)
{
	if (!ignore && ignore_incoming_packets) throw InvalidOperationException("Cannot stop ignoring packets");
	ignore_incoming_packets = ignore;
}

void PipeEndpointBase::AddListener(RR_SHARED_PTR<PipeEndpointBaseListener> listener)
{
	listeners.push_back(listener);
}

void PipeEndpointBase::Shutdown()
{
	closed = true;
	
	RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(&PipeEndpointBase::fire_PipeEndpointClosedCallback, shared_from_this()));

	std::list<RR_WEAK_PTR<PipeEndpointBaseListener> > listeners1;
	{
		listeners.swap(listeners1);
	}

	BOOST_FOREACH(RR_WEAK_PTR<PipeEndpointBaseListener> l, listeners1)
	{
		RR_SHARED_PTR<PipeEndpointBaseListener> l1 = l.lock();
		if (l1)
		{
			RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(&PipeEndpointBaseListener::PipeEndpointClosed, l1, shared_from_this()));
		}
	}
}

void PipeBase::DispatchPacketAck (RR_INTRUSIVE_PTR<MessageElement> me, RR_SHARED_PTR<PipeEndpointBase> e)
{
	uint32_t pnum;
	if (me->ElementFlags & MessageElementFlags_SEQUENCE_NUMBER)
	{
		pnum = me->SequenceNumber;
	}
	else
	{
		pnum = RRArrayToScalar(me->CastData<RRArray<uint32_t> >());
	}
	e->PipePacketAckReceived(pnum);
}

bool PipeBase::DispatchPacket(RR_INTRUSIVE_PTR<MessageElement> me, RR_SHARED_PTR<PipeEndpointBase> e, uint32_t& packetnumber)
{
	//int32_t index=boost::lexical_cast<int32_t>(me->ElementName);
	if ((me->ElementFlags & MessageElementFlags_ELEMENT_NUMBER) && (me->ElementFlags & MessageElementFlags_SEQUENCE_NUMBER))
	{
		//Use message 3
		packetnumber = me->SequenceNumber;
		RR_INTRUSIVE_PTR<RRValue> data;
		if (!rawelements)
		{
			data = UnpackData(me);
		}
		else
		{
			data = me;
		}

		e->PipePacketReceived(data, packetnumber);

		return (me->ElementFlags & MessageElementFlags_REQUEST_ACK)!=0;
	}
	else
	{
		//Use message 2
		RR_INTRUSIVE_PTR<MessageElementMap<std::string> > elems1 = me->CastData<MessageElementMap<std::string> >();
		packetnumber = RRArrayToScalar(MessageElement::FindElement(elems1->Elements, "packetnumber")->CastData<RRArray<uint32_t> >());

		RR_INTRUSIVE_PTR<RRValue> data;
		if (!rawelements)
		{
			data = UnpackData(MessageElement::FindElement(elems1->Elements, "packet"));
		}
		else
		{
			data = MessageElement::FindElement(elems1->Elements, "packet");
		}

		e->PipePacketReceived(data, packetnumber);

		RR_INTRUSIVE_PTR<MessageElement> e;
		if (MessageElement::TryFindElement(elems1->Elements, "requestack", e))
		{
			return true;
		}		
	}

	return false;

}

RR_INTRUSIVE_PTR<MessageElement> PipeBase::PackPacket(RR_INTRUSIVE_PTR<RRValue> data, int32_t index, uint32_t packetnumber, bool requestack, bool message3)
{	
	if (message3)
	{
		//Use message 3		
		RR_INTRUSIVE_PTR<MessageElement> me;
		if (!rawelements)
		{
			me = CreateMessageElement();
			me->SetData(PackData(data));
		}
		else
		{
			me = CreateMessageElement();
			RR_INTRUSIVE_PTR<MessageElement> me2 = rr_cast<MessageElement>(data);
			me->SetData(me2->GetData());
		}

		me->ElementFlags = MessageElementFlags_ELEMENT_NUMBER | MessageElementFlags_SEQUENCE_NUMBER;
		me->ElementNumber = index;
		me->SequenceNumber = packetnumber;

		if (requestack)
		{
			me->ElementFlags |= MessageElementFlags_REQUEST_ACK;
		}

		return me;
	}
	else
	{
		//Use message 2
		std::vector<RR_INTRUSIVE_PTR<MessageElement> > elems;
		elems.push_back(CreateMessageElement("packetnumber", ScalarToRRArray(packetnumber)));

		if (!rawelements)
		{
			RR_INTRUSIVE_PTR<MessageElementData> pdata = PackData(data);
			elems.push_back(CreateMessageElement("packet", pdata));
		}
		else
		{
			RR_INTRUSIVE_PTR<MessageElement> pme = rr_cast<MessageElement>(data);
			pme->ElementName = "packet";
			elems.push_back(pme);
		}

		if (requestack)
		{
			elems.push_back(CreateMessageElement("requestack", ScalarToRRArray(static_cast<uint32_t>(1))));
		}

		RR_INTRUSIVE_PTR<MessageElementMap<std::string> > delems = CreateMessageElementMap<std::string>(elems);
		RR_INTRUSIVE_PTR<MessageElement> me = CreateMessageElement(boost::lexical_cast<std::string>(index), delems);

		return me;
	}

}

RR_SHARED_PTR<RobotRaconteurNode> PipeBase::GetNode()
{
	RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
	if (!n) throw InvalidOperationException("Node has been released");
	return n;
}

MemberDefinition_Direction PipeBase::Direction()
{
	return direction;
}

bool PipeBase::IsUnreliable()
{
	return unreliable;
}

std::string PipeClientBase::GetMemberName()
{
	return m_MemberName;
}

void PipeClientBase::PipePacketReceived(RR_INTRUSIVE_PTR<MessageEntry> m, uint32_t e)
{
	//boost::shared_lock<boost::shared_mutex> lock(stub_lock);
	
	if (m->EntryType == MessageEntryType_PipeClosed)
	{
		try
		{
			

			int32_t index = RRArrayToScalar(m->FindElement("index")->CastData<RRArray<int32_t> >());
			RR_SHARED_PTR<PipeEndpointBase> p;
			{
				RR_UNORDERED_MAP<int32_t, RR_SHARED_PTR<PipeEndpointBase> >::iterator e1 = pipeendpoints.find(index);
				if (e1 == pipeendpoints.end()) return;
				p = e1->second;
				pipeendpoints.erase(e1);				
			}
			p->RemoteClose();
			
		}
		catch (std::exception&)
		{
		};
	}

	else if (m->EntryType==MessageEntryType_PipeClosedRet)
	{
		try
		{
			int32_t index = RRArrayToScalar(m->FindElement("index")->CastData<RRArray<int32_t> >());
			
			pipeendpoints.erase(index);
		}
		catch (std::exception&)
		{
		};	
	}

	else if (m->EntryType == MessageEntryType_PipePacket)
	{
		std::vector<RR_INTRUSIVE_PTR<MessageElement> > ack;;
		BOOST_FOREACH (RR_INTRUSIVE_PTR<MessageElement>& me, m->elements)
		{
			try
			{				
				int32_t index;
				if (me->ElementFlags & MessageElementFlags_ELEMENT_NUMBER)
				{
					index = me->ElementNumber;
				}
				else
				{
					index = boost::lexical_cast<int32_t>(me->ElementName);
				}
				uint32_t pnum;
				RR_SHARED_PTR<PipeEndpointBase> e;
				{
					RR_UNORDERED_MAP<int32_t, RR_SHARED_PTR<PipeEndpointBase> >::iterator e1;
					if ((e1=pipeendpoints.find(index)) != pipeendpoints.end())
					{
						e = e1->second;
					}
					else if ((e1=early_endpoints.find(index)) != early_endpoints.end())
					{
						e = e1->second;
					}
					else
					{
						if (connecting_endpoints.size() == 0)
						{
							continue;
						}

						bool found = false;

						typedef boost::tuple<int32_t, int32_t> e2_type;
						BOOST_FOREACH (e2_type& e2, connecting_endpoints)	
						{
							if (e2.get<1>() == index || e2.get<1>() == -1)
								found = true;
						}

						if (!found)
						{
							continue;
						}

						RR_SHARED_PTR<PipeEndpointBase> new_ep = CreateNewPipeEndpoint(index, false, direction, GetStub()->GetContext()->UseMessage3());
						early_endpoints.insert(std::make_pair(index, new_ep));
						e = new_ep;
					}					
				}

				if (DispatchPacket(me, e, pnum))
				{		
					if (me->ElementFlags & MessageElementFlags_ELEMENT_NUMBER)
					{
						RR_INTRUSIVE_PTR<MessageElement> me1 = CreateMessageElement();
						me1->SequenceNumber=(boost::numeric_cast<uint32_t>(pnum));
						me1->ElementNumber = me->ElementNumber;
						me1->ElementFlags = MessageElementFlags_ELEMENT_NUMBER | MessageElementFlags_SEQUENCE_NUMBER;
						ack.push_back(me1);
					}
					else
					{
						ack.push_back(CreateMessageElement(me->ElementName, ScalarToRRArray(boost::numeric_cast<uint32_t>(pnum))));
					}
				}
			}

			catch (std::exception& e)
			{
				RobotRaconteurNode::TryHandleException(node, &e);
			}

		}
		try
		{
			if (ack.size() > 0)
			{				
				RR_INTRUSIVE_PTR<MessageEntry> mack = CreateMessageEntry(MessageEntryType_PipePacketRet, m->MemberName);
				mack->elements = ack;
				if (!(ack.at(0)->ElementFlags & MessageElementFlags_ELEMENT_NUMBER))
				{
					if (unreliable)
					{
						mack->MetaData = "unreliable\n";
					}
				}
				boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)> h = boost::bind(&PipeMember_empty_handler, _1);
				GetStub()->AsyncSendPipeMessage(mack,unreliable,h);

			}
		}
		catch (std::exception& e)
		{
			RobotRaconteurNode::TryHandleException(node, &e);
		}
		}
		else if (m->EntryType == MessageEntryType_PipePacketRet)
		{

		try
		{
			

			BOOST_FOREACH (RR_INTRUSIVE_PTR<MessageElement>& me, m->elements)
			{
				int32_t index;
				if (me->ElementFlags & MessageElementFlags_ELEMENT_NUMBER)
				{
					index = me->ElementNumber;
				}
				else
				{
					index = boost::lexical_cast<int32_t>(me->ElementName);
				}

				RR_SHARED_PTR<PipeEndpointBase> e;
				{
					RR_UNORDERED_MAP<int32_t, RR_SHARED_PTR<PipeEndpointBase> >::iterator e1 = pipeendpoints.find(index);
					if (e1 == pipeendpoints.end()) return;
					e = e1->second;
				}
				DispatchPacketAck(me, e);
			}
		}
		catch (std::exception&)
		{
			
		}

	}
}

void PipeClientBase::Shutdown()
{
	std::vector<RR_SHARED_PTR<PipeEndpointBase> > p;
	{
		boost::copy(pipeendpoints | boost::adaptors::map_values, std::back_inserter(p));
		pipeendpoints.clear();
	}
	
	BOOST_FOREACH (RR_SHARED_PTR<PipeEndpointBase>& e, p)
	{
		try
		{		
		e->Shutdown();
		}
		catch (std::exception& e) 
		{
			RobotRaconteurNode::TryHandleException(node, &e);
		}		
	}

	//stub.reset();
}
		
void PipeClientBase::AsyncSendPipePacket(RR_INTRUSIVE_PTR<RRValue> data, int32_t index, uint32_t packetnumber, bool requestack, uint32_t endpoint, bool unreliable, bool message3, RR_MOVE_ARG(boost::function<void(uint32_t,RR_SHARED_PTR<RobotRaconteurException>)>) handler)
{	
	RR_INTRUSIVE_PTR<MessageElement> me = PackPacket(data, index, packetnumber, requestack, message3);
	RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_PipePacket, GetMemberName());
	m->AddElement(me);
	if (!message3)
	{
		if (unreliable)
			m->MetaData = "unreliable\n";
	}
	boost::function<void(RR_SHARED_PTR<RobotRaconteurException>)> h = boost::bind(handler, packetnumber, _1);
	GetStub()->AsyncSendPipeMessage(m,unreliable, h);
}

void PipeClientBase::AsyncClose(RR_SHARED_PTR<PipeEndpointBase> endpoint, bool remote, uint32_t ee, RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
{
	
	if (!remote)
	{
	RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_PipeDisconnectReq, GetMemberName());
	m->AddElement("index", ScalarToRRArray(endpoint->GetIndex()));
	GetStub()->AsyncProcessRequest(m,boost::bind(handler,_2),timeout);
	}
}


void PipeClientBase::AsyncConnect_internal(int32_t index, RR_MOVE_ARG(boost::function<void (RR_SHARED_PTR<PipeEndpointBase>,RR_SHARED_PTR<RobotRaconteurException>)>) handler, int32_t timeout)
{

	connecting_key_count = connecting_key_count < std::numeric_limits<int32_t>::max() ? connecting_key_count + 1 : 0;
	int32_t key = connecting_key_count;
	boost::tuple<int, int> entry = boost::make_tuple(key, index);
	connecting_endpoints.push_back(entry);

	RR_INTRUSIVE_PTR<MessageEntry> m = CreateMessageEntry(MessageEntryType_PipeConnectReq, GetMemberName());
	m->AddElement("index", ScalarToRRArray(index));

	if (unreliable)
		m->AddElement("unreliable",ScalarToRRArray(static_cast<int32_t>(1)));
	
	GetStub()->AsyncProcessRequest(m,boost::bind(&PipeClientBase::AsyncConnect_internal1, RR_DYNAMIC_POINTER_CAST<PipeClientBase>(shared_from_this()),_1,_2,index,key,handler),timeout);
}

void PipeClientBase::AsyncConnect_internal1(RR_INTRUSIVE_PTR<MessageEntry> ret, RR_SHARED_PTR<RobotRaconteurException> err, int32_t index, int32_t key, boost::function<void (RR_SHARED_PTR<PipeEndpointBase>,RR_SHARED_PTR<RobotRaconteurException>)>& handler)
{

	boost::tuple<int, int> k = boost::make_tuple(key, index);

	for (std::list<boost::tuple<int32_t, int32_t> >::iterator e2 = connecting_endpoints.begin(); e2 != connecting_endpoints.end();)
	{
		if (e2->get<0>() == k.get<0>() && e2->get<1>() == k.get<1>())
		{
			e2=connecting_endpoints.erase(e2);
		}
		else
		{
			e2++;
		}
	}

	if (err)
	{
		try
		{
			if (connecting_endpoints.size() == 0)
			{
				early_endpoints.clear();
			}
			
			detail::InvokeHandlerWithException(node, handler, err);
			return;
		}
		catch (std::exception& e) 
		{
			RobotRaconteurNode::TryHandleException(node, &e);
			return;
		}
	}

	try
	{
		bool runreliable=false;

		try
		{
			if (RRArrayToScalar((ret->FindElement("unreliable")->CastData<RRArray<int32_t> >()))==1)
				runreliable=true;
		}
		catch (std::exception&) 
		{
			
		}

		int32_t rindex = RRArrayToScalar((ret->FindElement("index")->CastData<RRArray<int32_t> >()));

		RR_SHARED_PTR<PipeEndpointBase> e;
		
		RR_UNORDERED_MAP<int32_t, RR_SHARED_PTR<PipeEndpointBase> >::iterator e1 = early_endpoints.find(rindex);
		if (!runreliable)
		{
			if (e1!=early_endpoints.end())
			{
				e = e1->second;
				early_endpoints.erase(e1);
			}
		}
		else
		{
			if (e1 != early_endpoints.end())
			{
				early_endpoints.erase(e1);
			}
		}

		if (!e)
		{
			e = CreateNewPipeEndpoint(rindex, runreliable, direction, GetStub()->GetContext()->UseMessage3());
		}
						
		pipeendpoints.insert(std::make_pair(rindex, e));
		
		try
		{
			if (connecting_endpoints.size() == 0)
			{
				early_endpoints.clear();
			}
			
			detail::InvokeHandler(node, handler, e);
		}
		catch (std::exception& exp) {
			RobotRaconteurNode::TryHandleException(node, &exp);
		}
	}
	catch (std::exception& err2)
	{
		if (connecting_endpoints.size() == 0)
		{
			early_endpoints.clear();
		}
		
		detail::InvokeHandlerWithException(node, handler, err2);		
	}

	
}

PipeClientBase::PipeClientBase(const std::string& name, RR_SHARED_PTR<ServiceStub> stub, bool unreliable, MemberDefinition_Direction direction)
{
	m_MemberName=name;
	this->stub=stub;
	this->unreliable=unreliable;
	this->direction = direction;
	this->node=stub->RRGetNode();	
	connecting_key_count = 0;
}

RR_SHARED_PTR<ServiceStub> PipeClientBase::GetStub()
{
	RR_SHARED_PTR<ServiceStub> out=stub.lock();
	if (!out) throw InvalidOperationException("Pipe has been closed");
	return out;
}

void PipeClientBase::DeleteEndpoint(RR_SHARED_PTR<PipeEndpointBase> e)
{
	pipeendpoints.erase(e->GetIndex());
}

}
