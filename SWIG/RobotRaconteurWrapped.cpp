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

#include "RobotRaconteurWrapped.h"
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>
//#include <Python.h>


namespace RobotRaconteur
{
#ifdef RR_PYTHON
	bool RRNativeDirectorSupport::running=false;
#endif
	//Wrapped Service Factory

	WrappedServiceFactory::WrappedServiceFactory(const std::string& defstring)
	{
		this->defstring=defstring;
		servicedef=RR_MAKE_SHARED<ServiceDefinition>();
		servicedef->FromString(defstring);
	}

	WrappedServiceFactory::WrappedServiceFactory(boost::shared_ptr<RobotRaconteur::ServiceDefinition> def)
	{
		this->defstring=def->ToString();
		servicedef=def;
		
	}

	std::string WrappedServiceFactory::GetServiceName()
	{
		return servicedef->Name;
	}

	std::string WrappedServiceFactory::DefString()
	{
		return defstring;
	}
	
	RR_SHARED_PTR<ServiceDefinition> WrappedServiceFactory::ServiceDef()
	{
		return servicedef;
	}

	RR_SHARED_PTR<RobotRaconteur::StructureStub> WrappedServiceFactory::FindStructureStub(const std::string& s)
	{
		throw ServiceException("Invalid for wrapped service type");
	}

	RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementStructure> WrappedServiceFactory::PackStructure(RR_INTRUSIVE_PTR<RobotRaconteur::RRStructure> structin)
	{
		throw ServiceException("Invalid for wrapped service type");
	}

	RR_INTRUSIVE_PTR<RobotRaconteur::RRValue> WrappedServiceFactory::UnpackStructure(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementStructure> mstructin)
	{
		throw ServiceException("Invalid for wrapped service type");
	}

	RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementPodArray> WrappedServiceFactory::PackPodArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseArray> WrappedServiceFactory::UnpackPodArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementPodArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementPodMultiDimArray> WrappedServiceFactory::PackPodMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseMultiDimArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseMultiDimArray> WrappedServiceFactory::UnpackPodMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementPodMultiDimArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	
	RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNamedArray> WrappedServiceFactory::PackNamedArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseArray> WrappedServiceFactory::UnpackNamedArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNamedArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNamedMultiDimArray> WrappedServiceFactory::PackNamedMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseMultiDimArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseMultiDimArray> WrappedServiceFactory::UnpackNamedMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNamedMultiDimArray> structure)
	{
		throw ServiceException("Invalid for wrapped service type");
	}
	
	RR_SHARED_PTR<RobotRaconteur::ServiceStub> WrappedServiceFactory::CreateStub(const std::string& type, const std::string& path, RR_SHARED_PTR<RobotRaconteur::ClientContext> context)
	{
		boost::tuple<std::string,std::string> res=SplitQualifiedName(type);
		
		std::string servicetype=res.get<0>();
		std::string objecttype=res.get<1>();
		if (servicetype != GetServiceName()) return RobotRaconteur::RobotRaconteurNode::s()->GetServiceType(servicetype)->CreateStub(type,path,context);
		for (std::vector<RR_SHARED_PTR<ServiceEntryDefinition> >::iterator ee=servicedef->Objects.begin(); ee!=servicedef->Objects.end(); ++ee)
		{
			if ((*ee)->Name==objecttype)
			{
				RR_SHARED_PTR<WrappedServiceStub> out=RR_MAKE_SHARED<WrappedServiceStub>(path,*ee,context);
				out->RRInitStub();
				return out;
			}
		}
		throw RobotRaconteur::ServiceException("Invalid service stub type.");
	}

	//Wrapped Dynamic Service Factory
	RR_SHARED_PTR<ServiceFactory> WrappedDynamicServiceFactory::CreateServiceFactory(const std::string& def)
	{
		return RR_MAKE_SHARED<WrappedServiceFactory>(def);
	}
	std::vector<RR_SHARED_PTR<ServiceFactory> > WrappedDynamicServiceFactory::CreateServiceFactories(const std::vector<std::string>& def)
	{
		std::vector<RR_SHARED_PTR<ServiceFactory> > out;
		for (std::vector<std::string>::const_iterator ee=def.begin(); ee!=def.end(); ++ee)
		{
			out.push_back(RR_MAKE_SHARED<WrappedServiceFactory>(*ee));
		}
		return out;
	}


	//Wrapped Service Stub
	WrappedServiceStub::WrappedServiceStub(const std::string& path, RR_SHARED_PTR<ServiceEntryDefinition> type, RR_SHARED_PTR<RobotRaconteur::ClientContext> c)
		: RobotRaconteur::ServiceStub(path,c)
	{
		RR_objecttype=type;
		//this->RR_Director=0;
		this->objectheapid=0;
#ifdef RR_PYTHON
		pystub=NULL;
		//DIRECTOR_CALL2(Py_XINCREF(pystub));
#endif 
	}

	void WrappedServiceStub::RRInitStub()
	{
		for (std::vector<RR_SHARED_PTR<MemberDefinition> >::iterator e=RR_objecttype->Members.begin(); e!=RR_objecttype->Members.end(); ++e)
		{
			RR_SHARED_PTR<PipeDefinition> p=boost::dynamic_pointer_cast<PipeDefinition>(*e);
			if (p)
			{
				bool unreliable=p->IsUnreliable();
				MemberDefinition_Direction direction = p->Direction();
				
				RR_SHARED_PTR<WrappedPipeClient> c=RR_MAKE_SHARED<WrappedPipeClient>(p->Name,shared_from_this(),p->Type, unreliable, direction);
				pipes.insert(std::make_pair(p->Name,c));				

			}

			RR_SHARED_PTR<WireDefinition> w=boost::dynamic_pointer_cast<WireDefinition>(*e);
			if (w)
			{
				MemberDefinition_Direction direction = w->Direction();
				RR_SHARED_PTR<WrappedWireClient> c=RR_MAKE_SHARED<WrappedWireClient>(w->Name,shared_from_this(),w->Type,direction);
				wires.insert(std::make_pair(w->Name,c));

			}
		}
	}

	void WrappedServiceStub::async_PropertyGet(const std::string& PropertyName, int32_t timeout, AsyncRequestDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncRequestDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncRequestDirector>, _1, id));
		RR_INTRUSIVE_PTR<MessageEntry> req=CreateMessageEntry(MessageEntryType_PropertyGetReq,PropertyName);
		AsyncProcessRequest(req,boost::bind(&WrappedServiceStub::async_PropertyGet_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);

	}
	void WrappedServiceStub::async_PropertySet(const std::string& PropertyName,  RR_INTRUSIVE_PTR<MessageElement> value, int32_t timeout, AsyncRequestDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncRequestDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncRequestDirector>, _1, id));
		RR_INTRUSIVE_PTR<MessageEntry> req=CreateMessageEntry(MessageEntryType_PropertySetReq, PropertyName);
		value->ElementName="value";
		req->AddElement(value);
		AsyncProcessRequest(req,boost::bind(&WrappedServiceStub::async_PropertySet_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);
	}
	void WrappedServiceStub::async_FunctionCall(const std::string& FunctionName, const std::vector<RR_INTRUSIVE_PTR<MessageElement> >& args, int32_t timeout, AsyncRequestDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncRequestDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncRequestDirector>, _1, id));
		RR_INTRUSIVE_PTR<MessageEntry> req=CreateMessageEntry(MessageEntryType_FunctionCallReq, FunctionName);
		req->elements=args;
		AsyncProcessRequest(req,boost::bind(&WrappedServiceStub::async_FunctionCall_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);
	}
	
	void WrappedServiceStub::async_GeneratorFunctionCall(const std::string& FunctionName, const std::vector<RR_INTRUSIVE_PTR<MessageElement> >& args, int32_t timeout, AsyncGeneratorClientReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncGeneratorClientReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncGeneratorClientReturnDirector>, _1, id));
		RR_INTRUSIVE_PTR<MessageEntry> req = CreateMessageEntry(MessageEntryType_FunctionCallReq, FunctionName);
		req->elements = args;
		AsyncProcessRequest(req, boost::bind(&WrappedServiceStub::async_GeneratorFunctionCall_handler, rr_cast<WrappedServiceStub>(shared_from_this()), FunctionName, _1, _2, sphandler), timeout);
	}

	void WrappedServiceStub::async_PropertyGet_handler( RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler)
	{
		
		if (err)
		{

		DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),err->ErrorCode,err->Error,err->Message));
		
		return;
		}
		if (m->Error != RobotRaconteur::MessageErrorType_None)
		{
			DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),m->Error,m->FindElement("errorname")->CastDataToString(),m->FindElement("errorstring")->CastDataToString()));
			return;
		}
		RR_INTRUSIVE_PTR<MessageElement> ret=m->FindElement("value");
		DIRECTOR_CALL2(handler->handler(ret,0,"",""));
	}

	void WrappedServiceStub::async_PropertySet_handler(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler)
	{
		
		if (err)
		{

		DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),err->ErrorCode,err->Error,err->Message));
		
		return;
		}
		if (m->Error != RobotRaconteur::MessageErrorType_None)
		{
			DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),m->Error,m->FindElement("errorname")->CastDataToString(),m->FindElement("errorstring")->CastDataToString()));
			return;
		}

		DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),0,"",""));
	}

	void WrappedServiceStub::async_FunctionCall_handler(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler)
	{
		
		if (err)
		{

		DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),err->ErrorCode,err->Error,err->Message));
		
		return;
		}
		if (m->Error != RobotRaconteur::MessageErrorType_None)
		{
			DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(),m->Error,m->FindElement("errorname")->CastDataToString(),m->FindElement("errorstring")->CastDataToString()));
			return;
		}

		RR_INTRUSIVE_PTR<MessageElement> ret;

		try
		{
			ret=m->FindElement("return");
		}
		catch (std::exception&) {}

		DIRECTOR_CALL2(handler->handler(ret,0,"",""));
	}

	void WrappedServiceStub::async_GeneratorFunctionCall_handler(const std::string& FunctionName, RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncGeneratorClientReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(RR_SHARED_PTR<WrappedGeneratorClient>(), err->ErrorCode, err->Error, err->Message));
			return;
		}
		if (m->Error != RobotRaconteur::MessageErrorType_None)
		{
			DIRECTOR_CALL2(handler->handler(RR_SHARED_PTR<WrappedGeneratorClient>(), m->Error, m->FindElement("errorname")->CastDataToString(), m->FindElement("errorstring")->CastDataToString()));
			return;
		}

		RR_INTRUSIVE_PTR<MessageElement> ret;

		try
		{
			ret = m->FindElement("return");
		}
		catch (std::exception&) {}

		RR_SHARED_PTR<WrappedGeneratorClient> gen_ret = RR_MAKE_SHARED<WrappedGeneratorClient>(FunctionName, RRArrayToScalar(m->FindElement("index")->CastData<RRArray<int32_t> >()), shared_from_this());

		DIRECTOR_CALL2(handler->handler(gen_ret, 0, "", ""));
	}

	void WrappedServiceStub::async_FindObjRef(const std::string& path, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncStubReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncStubReturnDirector>, _1, id));
		AsyncFindObjRef(path,boost::bind(&WrappedServiceStub::async_FindObjRef_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);
	}

	void WrappedServiceStub::async_FindObjRef(const std::string& path, const std::string& ind, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncStubReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncStubReturnDirector>, _1, id));
		AsyncFindObjRef(path,ind,boost::bind(&WrappedServiceStub::async_FindObjRef_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);
	}

	void WrappedServiceStub::async_FindObjRefTyped(const std::string& path, const std::string& type, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncStubReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncStubReturnDirector>, _1, id));
		AsyncFindObjRefTyped(path,type,boost::bind(&WrappedServiceStub::async_FindObjRef_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);
	}

	void WrappedServiceStub::async_FindObjRefTyped(const std::string& path, const std::string& ind, const std::string& type, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncStubReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncStubReturnDirector>, _1, id));
		AsyncFindObjRefTyped(path,ind,type, boost::bind(&WrappedServiceStub::async_FindObjRef_handler,rr_cast<WrappedServiceStub>(shared_from_this()),_1,_2,sphandler),timeout);
	}

	void WrappedServiceStub::async_FindObjRef_handler(RR_SHARED_PTR<RRObject> stub, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncStubReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(RR_SHARED_PTR<WrappedServiceStub>(),err->ErrorCode,err->Error,err->Message));
			return;
		}

		RR_SHARED_PTR<WrappedServiceStub> stub2=rr_cast<WrappedServiceStub>(stub);


		DIRECTOR_CALL2(handler->handler(stub2,0,"",""));
	}
		


	void WrappedServiceStub::DispatchEvent(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m)
	{
		DIRECTOR_CALL3(WrappedServiceStubDirector,RR_Director2->DispatchEvent(m->MemberName,m->elements));			
		
	}

	void WrappedServiceStub::DispatchPipeMessage(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m)
	{
		std::map<std::string, RR_SHARED_PTR<WrappedPipeClient> >::iterator e = pipes.find(m->MemberName);
		if (e == pipes.end()) throw MemberNotFoundException("Pipe Member Not Found");
		e->second->PipePacketReceived(m);		
	}
	
	void WrappedServiceStub::DispatchWireMessage(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m)
	{
		std::map<std::string, RR_SHARED_PTR<WrappedWireClient> >::iterator e = wires.find(m->MemberName);
		if (e == wires.end()) throw MemberNotFoundException("Pipe Member Not Found");
		e->second->WirePacketReceived(m);
	}

	RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> WrappedServiceStub::CallbackCall(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m)
	{		
		RR_SHARED_PTR<CallbackDefinition> d;

		for (std::vector<RR_SHARED_PTR<MemberDefinition> >::iterator e=RR_objecttype->Members.begin(); e!=RR_objecttype->Members.end(); ++e)
		{
			if ((*e)->Name==m->MemberName)
				d=boost::dynamic_pointer_cast<CallbackDefinition>(*e);

		}

		if (!d) throw MemberNotFoundException("Member not found");

		RR_INTRUSIVE_PTR<MessageEntry> req=CreateMessageEntry(MessageEntryType_CallbackCallRet,m->MemberName);
		RR_INTRUSIVE_PTR<MessageElement> mres;
		try
		{
			DIRECTOR_CALL(WrappedServiceStubDirector,mres=RR_Director2->CallbackCall(m->MemberName,m->elements));
		}
		catch (std::exception&)
		{
			throw;
		}
		catch (...)
		{

			throw UnknownException("RobotRaconteur.UnknownException", "Error occured in callback");

		}
		
		if (!mres)
		{			
			throw OperationFailedException("Exception occured in callback");			
		}
		
		RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> res=CreateMessageEntry( RobotRaconteur::MessageEntryType_CallbackCallRet,m->MemberName);
		res->ServicePath=m->ServicePath;
		res->RequestID=m->RequestID;
		if (d->ReturnType->Type != DataTypes_void_t)
		{
			mres->ElementName="return";
			res->AddElement(mres);
		}
		else
		{
			res->AddElement("return",RobotRaconteur::ScalarToRRArray<int32_t>(0));
		}

		return res;		
	}

	RR_SHARED_PTR<RobotRaconteur::WrappedPipeClient> WrappedServiceStub::GetPipe(const std::string& membername)
	{
		std::map<std::string, RR_SHARED_PTR<WrappedPipeClient> >::iterator e = pipes.find(membername);
		if (e == pipes.end()) throw MemberNotFoundException("Pipe Member Not Found");
		return e->second;
	}

	RR_SHARED_PTR<RobotRaconteur::WrappedWireClient> WrappedServiceStub::GetWire(const std::string& membername)
	{
		std::map<std::string, RR_SHARED_PTR<WrappedWireClient> >::iterator e = wires.find(membername);
		if (e == wires.end()) throw MemberNotFoundException("Wire Member Not Found");
		return e->second;
	}

	RR_SHARED_PTR<PipeClientBase> WrappedServiceStub::RRGetPipeClient(const std::string& membername)
	{
		return GetPipe(membername);
	}

	RR_SHARED_PTR<WireClientBase> WrappedServiceStub::RRGetWireClient(const std::string& membername)
	{
		return GetWire(membername);
	}

	void WrappedServiceStub::RRClose()
	{
		//DIRECTOR_CALL2(DIRECTOR_DELETE( RR_Director));

		for (std::map<std::string, RR_SHARED_PTR<WrappedPipeClient> >::iterator e=pipes.begin(); e!=pipes.end(); ++e)
		{
			e->second->Shutdown();
		}

		for (std::map<std::string, RR_SHARED_PTR<WrappedWireClient> >::iterator e=wires.begin(); e!=wires.end(); ++e)
		{
			e->second->Shutdown();
		}


		//TODO: fill this in
		ServiceStub::RRClose();

		RR_Director.reset();

#ifdef RR_PYTHON
		if (pystub!=NULL)
		{
			DIRECTOR_CALL2(Py_XDECREF(pystub));
			pystub=NULL;
			
		}
#else
		//RR_Director=NULL;
#endif

	}

	std::string WrappedServiceStub::RRType()
	{
		return RR_objecttype->Name;
	}

	WrappedServiceStub::~WrappedServiceStub()
	{
		//DIRECTOR_CALL2(DIRECTOR_DELETE( this->RR_Director));
#ifdef RR_PYTHON
		//DIRECTOR_CALL2(Py_XDECREF(pystub));
#endif

	}

	/*WrappedServiceStubDirector* WrappedServiceStub::GetRRDirector()
	{
		return RR_Director;
	}*/

	void WrappedServiceStub::SetRRDirector(WrappedServiceStubDirector* director, int32_t id)
	{
		objectheapid=id;
		this->RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedServiceStubDirector>, _1, id));
	}
	
	int WrappedServiceStub::GetObjectHeapID()
	{
		return objectheapid;
	}

	//Wrapped Pipe Endpoint

	void WrappedPipeEndpoint::AsyncSendPacket(RR_INTRUSIVE_PTR<MessageElement> packet, AsyncUInt32ReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncUInt32ReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncUInt32ReturnDirector>, _1, id));
		AsyncSendPacketBase(rr_cast<RRValue>(packet),boost::bind(&WrappedPipeEndpoint::AsyncSendPacket_handler,rr_cast<WrappedPipeEndpoint>(shared_from_this()),_1,_2,sphandler));
	}

	void WrappedPipeEndpoint::AsyncSendPacket_handler(uint32_t id, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncUInt32ReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(0,err->ErrorCode,err->Error,err->Message));
			return;
		}

		DIRECTOR_CALL2(handler->handler(id,0,"",""));
	}
	

	RR_INTRUSIVE_PTR<MessageElement> WrappedPipeEndpoint::ReceivePacket()
	{
		return rr_cast<MessageElement>(ReceivePacketBase());
	}

	RR_INTRUSIVE_PTR<MessageElement> WrappedPipeEndpoint::PeekNextPacket()
	{
		return rr_cast<MessageElement>(PeekPacketBase());
	}
	
	bool WrappedPipeEndpoint::TryReceivePacket(RR_INTRUSIVE_PTR<MessageElement>& packet, bool peek)
	{
		RR_INTRUSIVE_PTR<RRValue> o;
		if (!TryReceivePacketBase(o, peek))
		{
			return false;
		}

		packet = rr_cast<MessageElement>(o);
		return true;
	}


	WrappedPipeEndpoint::WrappedPipeEndpoint(RR_SHARED_PTR<PipeBase> parent, int32_t index, uint32_t endpoint, RR_SHARED_PTR<TypeDefinition> type, bool unreliable, MemberDefinition_Direction direction, bool message3)
		: PipeEndpointBase(parent,index,endpoint,unreliable,direction,message3) {
		this->Type=type;
		//this->RR_Director=0;
		//this->objectheapid=0;

	}

	void WrappedPipeEndpoint::fire_PipeEndpointClosedCallback()
	{
		
		DIRECTOR_CALL3(WrappedPipeEndpointDirector,RR_Director2->PipeEndpointClosedCallback());
			

	}

	void WrappedPipeEndpoint::fire_PacketReceivedEvent()
	{
		DIRECTOR_CALL3(WrappedPipeEndpointDirector,RR_Director2->PacketReceivedEvent());
				
	}

	void WrappedPipeEndpoint::fire_PacketAckReceivedEvent(uint32_t packetnum)
	{
			

		DIRECTOR_CALL3(WrappedPipeEndpointDirector,RR_Director2->PacketAckReceivedEvent(packetnum));
		
	}

	//WrappedPipeClient

	/*boost::function<void(RR_SHARED_PTR<WrappedPipeEndpoint>)> WrappedPipeClient::GetPipeConnectCallback()
	{
		throw InvalidOperationException("Not valid for client");
	}
		
	void WrappedPipeClient::SetPipeConnectCallback(boost::function<void(RR_SHARED_PTR<WrappedPipeEndpoint>)> function)
	{
		throw InvalidOperationException("Not valid for client");
	}*/
	
	/*WrappedPipeEndpointDirector* WrappedPipeEndpoint::GetRRDirector()
	{
		return RR_Director;
	}*/
	void WrappedPipeEndpoint::SetRRDirector(WrappedPipeEndpointDirector* director, int32_t id)
	{
		this->RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedPipeEndpointDirector>, _1, id));
	}

	RR_SHARED_PTR<WrappedServiceStub> WrappedPipeEndpoint::GetStub()
	{
		RR_SHARED_PTR<PipeBase> p = parent.lock();
		if (!p) return RR_SHARED_PTR<WrappedServiceStub>();
		RR_SHARED_PTR<PipeClientBase> p1=RR_DYNAMIC_POINTER_CAST<PipeClientBase>(p);
		if (!p1) return RR_SHARED_PTR<WrappedServiceStub>();
		return RR_DYNAMIC_POINTER_CAST<WrappedServiceStub>(p1->GetStub());
	}

	void WrappedPipeEndpoint::AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncVoidReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncVoidReturnDirector>, _1, id));

		{
		RR_Director.reset();
		}

		PipeEndpointBase::AsyncClose(boost::bind(&WrappedPipeEndpoint::AsyncClose_handler,rr_cast<WrappedPipeEndpoint>(shared_from_this()),_1,sphandler),timeout);


	}

	void WrappedPipeEndpoint::AsyncClose_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(err->ErrorCode,err->Error,err->Message));
			return;
		}

		DIRECTOR_CALL2(handler->handler(0,"",""));
	}


	WrappedPipeEndpoint::~WrappedPipeEndpoint()
	{
		//DIRECTOR_CALL2(DIRECTOR_DELETE(RR_Director));
	}

	
	void WrappedPipeClient::AsyncConnect(int32_t index, int32_t timeout, AsyncPipeEndpointReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncPipeEndpointReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncPipeEndpointReturnDirector>, _1, id));
		AsyncConnect_internal(index,boost::bind(&WrappedPipeClient::AsyncConnect_handler,rr_cast<WrappedPipeClient>(shared_from_this()),_1,_2,sphandler),timeout);
	}

	void WrappedPipeClient::AsyncConnect_handler(RR_SHARED_PTR<PipeEndpointBase> ep, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncPipeEndpointReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(RR_SHARED_PTR<WrappedPipeEndpoint>(),err->ErrorCode,err->Error,err->Message));
			return;
		}

		RR_SHARED_PTR<WrappedPipeEndpoint> ep2=boost::dynamic_pointer_cast<WrappedPipeEndpoint>(ep);

		DIRECTOR_CALL2(handler->handler(ep2,0,"",""));

	}

	WrappedPipeClient::WrappedPipeClient(const std::string& name, RR_SHARED_PTR<ServiceStub> stub, RR_SHARED_PTR<TypeDefinition> Type, bool unreliable, MemberDefinition_Direction direction) : PipeClientBase(name,stub,unreliable,direction)
	{
		this->Type=Type;
		this->rawelements=true;
	}	
	
	RR_SHARED_PTR<PipeEndpointBase> WrappedPipeClient::CreateNewPipeEndpoint(int32_t index, bool unreliable, MemberDefinition_Direction direction, bool message3)
	{
		return RR_MAKE_SHARED<WrappedPipeEndpoint>(rr_cast<WrappedPipeClient>(shared_from_this()),index,0,Type,unreliable,direction,message3);
	}
	
	//WrappedWireConnection
	RR_INTRUSIVE_PTR<MessageElement> WrappedWireConnection::GetInValue()
	{
		return RRPrimUtil<RR_INTRUSIVE_PTR<MessageElement> >::PreUnpack(GetInValueBase());
	}

	RR_INTRUSIVE_PTR<MessageElement> WrappedWireConnection::GetOutValue()
	{
		return RRPrimUtil<RR_INTRUSIVE_PTR<MessageElement> >::PreUnpack(GetOutValueBase());
	}

	void WrappedWireConnection::SetOutValue(RR_INTRUSIVE_PTR<MessageElement> value)
	{
		SetOutValueBase(RRPrimUtil<RR_INTRUSIVE_PTR<MessageElement> >::PrePack(value));
	}

	WrappedWireConnection::WrappedWireConnection(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint, RR_SHARED_PTR<TypeDefinition> Type, MemberDefinition_Direction direction, bool message3)
		: WireConnectionBase(parent,endpoint,direction,message3) 
	{
		this->Type=Type;
		//this->RR_Director=0;
		//this->objectheapid=0;
	
	}

	void WrappedWireConnection::fire_WireValueChanged(RR_INTRUSIVE_PTR<RRValue> value, TimeSpec time)
	{
		
		RR_INTRUSIVE_PTR<MessageElement> m=RRPrimUtil<RR_INTRUSIVE_PTR<MessageElement> >::PreUnpack(value);
		DIRECTOR_CALL3(WrappedWireConnectionDirector,RR_Director2->WireValueChanged(m,time));

		
	}

	void WrappedWireConnection::fire_WireClosedCallback()
	{
		
		DIRECTOR_CALL3(WrappedWireConnectionDirector,RR_Director2->WireConnectionClosedCallback());
				
	}

	/*WrappedWireConnectionDirector* WrappedWireConnection::GetRRDirector()
	{
		return RR_Director;
	}*/

	void WrappedWireConnection::SetRRDirector(WrappedWireConnectionDirector* director, int32_t id)
	{
		this->RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedWireConnectionDirector>, _1, id));
	}

	RR_SHARED_PTR<WrappedServiceStub> WrappedWireConnection::GetStub()
	{
		RR_SHARED_PTR<WireBase> w = parent.lock();
		if (!w) return RR_SHARED_PTR<WrappedServiceStub>();
		RR_SHARED_PTR<WireClientBase> w2 = RR_DYNAMIC_POINTER_CAST<WireClientBase>(w);
		if (!w2) return RR_SHARED_PTR<WrappedServiceStub>();
		return RR_DYNAMIC_POINTER_CAST<WrappedServiceStub>(w2->GetStub());
	}

	WrappedWireConnection::~WrappedWireConnection()
	{
		//DIRECTOR_CALL2(DIRECTOR_DELETE(RR_Director));
	}
	
	void WrappedWireConnection::AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncVoidReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncVoidReturnDirector>, _1, id));

		{
		RR_Director.reset();
		}

		WireConnectionBase::AsyncClose(boost::bind(&WrappedWireConnection::AsyncClose_handler,rr_cast<WrappedWireConnection>(shared_from_this()),_1,sphandler),timeout);


	}

	void WrappedWireConnection::AsyncClose_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(err->ErrorCode,err->Error,err->Message));
			return;
		}

		DIRECTOR_CALL2(handler->handler(0,"",""));
	}
	
	bool WrappedWireConnection::TryGetInValue(RR_INTRUSIVE_PTR<MessageElement>& value, TimeSpec& ts)
	{
		RR_INTRUSIVE_PTR<RRValue> value1;
		if (!TryGetInValueBase(value1, ts))
		{
			return false;
		}

		value = RR_DYNAMIC_POINTER_CAST<MessageElement>(value1);
		return true;
	}

	bool WrappedWireConnection::TryGetOutValue(RR_INTRUSIVE_PTR<MessageElement>& value, TimeSpec& ts)
	{
		RR_INTRUSIVE_PTR<RRValue> value1;
		if (!TryGetOutValueBase(value1, ts))
		{
			return false;
		}

		value = RR_DYNAMIC_POINTER_CAST<MessageElement>(value1);
		return true;
	}

	//WrappedWireClient

	void WrappedWireClient::AsyncConnect(int32_t timeout, AsyncWireConnectionReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncWireConnectionReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncWireConnectionReturnDirector>, _1, id));
		AsyncConnect_internal(boost::bind(&WrappedWireClient::AsyncConnect_handler,rr_cast<WrappedWireClient>(shared_from_this()),_1,_2,sphandler),timeout);
	}

	void WrappedWireClient::AsyncConnect_handler(RR_SHARED_PTR<WireConnectionBase> ep, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncWireConnectionReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(RR_SHARED_PTR<WrappedWireConnection>(),err->ErrorCode,err->Error,err->Message));
			return;
		}

		RR_SHARED_PTR<WrappedWireConnection> ep2=boost::dynamic_pointer_cast<WrappedWireConnection>(ep);

		DIRECTOR_CALL2(handler->handler(ep2,0,"",""));

	}

	WrappedWireClient::WrappedWireClient(const std::string& name, RR_SHARED_PTR<ServiceStub> stub, RR_SHARED_PTR<TypeDefinition> Type, MemberDefinition_Direction direction) : WireClientBase(name,stub,direction)
	{
		this->Type=Type;
		this->rawelements=true;
	}

	RR_SHARED_PTR<WireConnectionBase> WrappedWireClient::CreateNewWireConnection(MemberDefinition_Direction direction,bool message3)
	{
		return RR_MAKE_SHARED<WrappedWireConnection>(rr_cast<WrappedWireClient>(shared_from_this()),0,Type,direction,message3);
	}
	
	void WrappedWireClient::AsyncPeekInValue(int32_t timeout, AsyncWirePeekReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncWirePeekReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncWirePeekReturnDirector>, _1, id));
		AsyncPeekInValueBase(boost::bind(&WrappedWireClient::AsyncPeekValue_handler, rr_cast<WrappedWireClient>(shared_from_this()), _1, _2, _3, sphandler), timeout);
	}
	void WrappedWireClient::AsyncPeekOutValue(int32_t timeout, AsyncWirePeekReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncWirePeekReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncWirePeekReturnDirector>, _1, id));
		AsyncPeekOutValueBase(boost::bind(&WrappedWireClient::AsyncPeekValue_handler, rr_cast<WrappedWireClient>(shared_from_this()), _1, _2, _3, sphandler), timeout);
	}
	void WrappedWireClient::AsyncPokeOutValue(const RR_INTRUSIVE_PTR<MessageElement>& value, int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncVoidReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncVoidReturnDirector>, _1, id));
		AsyncPokeOutValueBase(value, boost::bind(&WrappedWireClient::AsyncPokeValue_handler, rr_cast<WrappedWireClient>(shared_from_this()), _1, sphandler), timeout);
	}

	void WrappedWireClient::AsyncPeekValue_handler(const RR_INTRUSIVE_PTR<RRValue>& value, const TimeSpec& ts, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncWirePeekReturnDirector> handler)
	{
		if (err)
		{
			RR_INTRUSIVE_PTR<MessageElement> el;
			TimeSpec ts;
			DIRECTOR_CALL2(handler->handler(el, ts, err->ErrorCode, err->Error, err->Message));
			return;
		}
		
		RR_INTRUSIVE_PTR<MessageElement> value2 = RR_DYNAMIC_POINTER_CAST<MessageElement>(value);
		DIRECTOR_CALL2(handler->handler(value2, ts, 0, "", ""));
	}
	void WrappedWireClient::AsyncPokeValue_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler)
	{
		if (err)
		{			
			DIRECTOR_CALL2(handler->handler(err->ErrorCode, err->Error, err->Message));
			return;
		}
				
		DIRECTOR_CALL2(handler->handler(0, "", ""));
	}

	//Generator Function

	WrappedGeneratorClient::WrappedGeneratorClient(const std::string& name, int32_t id, RR_SHARED_PTR<ServiceStub> stub)
		: GeneratorClientBase(name, id, stub)
	{

	}

	void WrappedGeneratorClient::AsyncNext(RR_INTRUSIVE_PTR<MessageElement> v, int32_t timeout, AsyncRequestDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncRequestDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncRequestDirector>, _1, id));
		AsyncNextBase(v, boost::bind(&WrappedGeneratorClient::AsyncNext_handler,  _1, _2, sphandler), timeout);
	}

	void WrappedGeneratorClient::AsyncNext_handler(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElement> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler)
	{
		if (err)
		{

			DIRECTOR_CALL2(handler->handler(RR_INTRUSIVE_PTR<MessageElement>(), err->ErrorCode, err->Error, err->Message));

			return;
		}		
		DIRECTOR_CALL2(handler->handler(m, 0, "", ""));
	}
		
	void WrappedGeneratorClient::AsyncAbort(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncVoidReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncVoidReturnDirector>, _1, id));
		GeneratorClientBase::AsyncAbort(boost::bind(&WrappedGeneratorClient::AsyncAbort_handler, _1, sphandler), timeout);
	}

	void WrappedGeneratorClient::AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncVoidReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncVoidReturnDirector>, _1, id));
		GeneratorClientBase::AsyncClose(boost::bind(&WrappedGeneratorClient::AsyncAbort_handler, _1, sphandler), timeout);
	}

	void WrappedGeneratorClient::AsyncAbort_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(err->ErrorCode, err->Error, err->Message));
			return;
		}

		DIRECTOR_CALL2(handler->handler(0, "", ""));
	}

	

	// Service Discovery
	ServiceInfo2Wrapped::ServiceInfo2Wrapped(const ServiceInfo2& value)
	{
		Name=value.Name;
		RootObjectType=value.RootObjectType;
		RootObjectImplements=value.RootObjectImplements;
		ConnectionURL=value.ConnectionURL;
		NodeID=value.NodeID;
		NodeName=value.NodeName;

		RR_INTRUSIVE_PTR<RRMap<std::string,RRValue> > map=AllocateEmptyRRMap<std::string,RRValue>();
		map->GetStorageContainer()=value.Attributes;
		RR_INTRUSIVE_PTR<MessageElementMap<std::string> > mmap=RobotRaconteurNode::s()->PackMapType<std::string,RRValue>(map);
		Attributes= CreateMessageElement("value",mmap);

	}

	void AsyncServiceInfo2VectorReturn_handler(RR_SHARED_PTR<std::vector<ServiceInfo2> > ret, RR_SHARED_PTR<AsyncServiceInfo2VectorReturnDirector> handler)
	{
		std::vector<ServiceInfo2Wrapped> ret1;
		if (ret)
		{
			for(std::vector<ServiceInfo2>::iterator e=ret->begin(); e!=ret->end(); e++)
			{
				ret1.push_back(ServiceInfo2Wrapped(*e));
			}

		}
		
		DIRECTOR_CALL2(handler->handler(ret1));
		return;
	}

	void AsyncNodeInfo2VectorReturn_handler(RR_SHARED_PTR<std::vector<NodeInfo2> > ret, RR_SHARED_PTR<AsyncNodeInfo2VectorReturnDirector> handler)
	{
		std::vector<NodeInfo2> ret1=*ret.get();
		
		DIRECTOR_CALL2(handler->handler(ret1));
		return;
	}

	void AsyncWrappedFindServiceByType(RR_SHARED_PTR<RobotRaconteurNode> node, const std::string &servicetype, const std::vector<std::string>& transportschemes, int32_t timeout, AsyncServiceInfo2VectorReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncServiceInfo2VectorReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector <AsyncServiceInfo2VectorReturnDirector>, _1, id));
		node->AsyncFindServiceByType(servicetype,transportschemes,boost::bind(&AsyncServiceInfo2VectorReturn_handler,_1,sphandler),timeout);
	}

	void AsyncWrappedFindNodeByID(RR_SHARED_PTR<RobotRaconteurNode> node, const NodeID& id, const std::vector<std::string>& transportschemes, int32_t timeout, AsyncNodeInfo2VectorReturnDirector* handler, int32_t id1)
	{
		RR_SHARED_PTR<AsyncNodeInfo2VectorReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncNodeInfo2VectorReturnDirector>, _1, id1));
		node->AsyncFindNodeByID(id,transportschemes,boost::bind(&AsyncNodeInfo2VectorReturn_handler,_1,sphandler),timeout);
	}
	
	void AsyncWrappedFindNodeByName(RR_SHARED_PTR<RobotRaconteurNode> node, const std::string& name, const std::vector<std::string>& transportschemes, int32_t timeout, AsyncNodeInfo2VectorReturnDirector* handler, int32_t id)
	{
		RR_SHARED_PTR<AsyncNodeInfo2VectorReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncNodeInfo2VectorReturnDirector>, _1, id));
		node->AsyncFindNodeByName(name,transportschemes,boost::bind(&AsyncNodeInfo2VectorReturn_handler,_1,sphandler),timeout);
	}
	
	void AsyncVoidNoErrReturn_handler(RR_SHARED_PTR<AsyncVoidNoErrReturnDirector> handler)
	{
		DIRECTOR_CALL2(handler->handler());
	}

	void AsyncWrappedUpdateDetectedNodes(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& schemes, int32_t timeout, AsyncVoidNoErrReturnDirector* handler, int32_t id1)
	{
		RR_SHARED_PTR<AsyncVoidNoErrReturnDirector> sphandler(handler, boost::bind(&ReleaseDirector<AsyncVoidNoErrReturnDirector>, _1, id1));
		node->AsyncUpdateDetectedNodes(schemes, boost::bind(&AsyncVoidNoErrReturn_handler, sphandler), timeout);
	}

	std::vector<std::string> WrappedGetDetectedNodes(RR_SHARED_PTR<RobotRaconteurNode> node)
	{
		std::vector<std::string> o;
		std::vector<NodeDiscoveryInfo> o1=node->GetDetectedNodes();
		for (std::vector<NodeDiscoveryInfo>::iterator e = o1.begin(); e != o1.end(); e++)
		{
			o.push_back(e->NodeID.ToString());
		}
		return o;
	}

	

	RR_INTRUSIVE_PTR<MessageEntry> RRDirectorExceptionHelper::last_err;

	void RRDirectorExceptionHelper::Reset()
	{
		last_err.reset();
	}
	void RRDirectorExceptionHelper::SetError(RR_INTRUSIVE_PTR<MessageEntry> err)
	{
		last_err = err;
	}
	
	bool RRDirectorExceptionHelper::IsErrorPending()
	{
		return last_err.get()!=NULL;
	}
	
	RR_INTRUSIVE_PTR<MessageEntry> RRDirectorExceptionHelper::GetError()
	{
		return last_err;
	}


    void AsyncStubReturn_handler(RR_SHARED_PTR<RRObject> obj, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncStubReturnDirector> handler)
	{
		if (err)
		{
			DIRECTOR_CALL2(handler->handler(RR_SHARED_PTR<WrappedServiceStub>(),err->ErrorCode,err->Error,err->Message));
			return;
		}

		RR_SHARED_PTR<WrappedServiceStub> stub=boost::dynamic_pointer_cast<WrappedServiceStub>(obj);

		DIRECTOR_CALL2(handler->handler(stub,0,"",""));
	}

	

	void AsyncStringReturn_handler(RR_SHARED_PTR<std::string> str, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncStringReturnDirector> handler)
	{
		if (err)
		{
			std::string ret = "";
			DIRECTOR_CALL2(handler->handler(ret,err->ErrorCode,err->Error,err->Message));
			return;
		}

		std::string* str2=str.get();

		DIRECTOR_CALL2(handler->handler(*str2,0,"",""));
	}

	void WrappedExceptionHandler(const std::exception* err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler)
	{
		if (err==NULL) return;
		try
		{
			const RobotRaconteurException* rrerr=dynamic_cast<const RobotRaconteurException*>(err);
			if (rrerr)
			{
				DIRECTOR_CALL2(handler->handler(rrerr->ErrorCode,rrerr->Error,rrerr->Message));
			}
			else
			{
				DIRECTOR_CALL2(handler->handler(MessageErrorType_UnknownError,std::string(typeid(*err).name()),err->what()));
			}
		}
		catch (std::exception&) {}

	}

	void TimerHandlerFunc(const TimerEvent& ev, RR_SHARED_PTR<AsyncTimerEventReturnDirector> d)
	{
		TimerEvent ev2=ev;
		DIRECTOR_CALL2(d->handler(ev2,0,"","");)
	}

	// Subscriptions

	bool WrappedServiceSubscriptionFilterPredicateDirector::CallPredicate(const ServiceInfo2& info)
	{
		ServiceInfo2Wrapped info2(info);

		bool res;
		DIRECTOR_CALL2(res = this->Predicate(info2));
		return res;
	}

	void WrappedServiceSubscriptionFilter::SetRRPredicateDirector(WrappedServiceSubscriptionFilterPredicateDirector* director, int32_t id)
	{
		Predicate.reset(director, boost::bind(&ReleaseDirector<WrappedServiceSubscriptionFilterPredicateDirector>, _1, id));
	}


	WrappedServiceInfo2Subscription::WrappedServiceInfo2Subscription(RR_SHARED_PTR<ServiceInfo2Subscription> subscription)
	{
		this->subscription = subscription;
	}

	std::map<ServiceSubscriptionClientID, ServiceInfo2Wrapped > WrappedServiceInfo2Subscription::GetDetectedServiceInfo2()
	{

		std::map<ServiceSubscriptionClientID, ServiceInfo2> a = subscription->GetDetectedServiceInfo2();
		std::map<ServiceSubscriptionClientID, ServiceInfo2Wrapped > b;

		typedef std::map<ServiceSubscriptionClientID, ServiceInfo2>::value_type e_type;

		BOOST_FOREACH(const e_type& e, a)
		{
			b.insert(std::make_pair(e.first, ServiceInfo2Wrapped(e.second)));
		}

		return b;
	}

	void WrappedServiceInfo2Subscription::Close()
	{
		{
			RR_Director.reset();
		}

		subscription->Close();
	}

	void WrappedServiceInfo2Subscription::SetRRDirector(WrappedServiceInfo2SubscriptionDirector* director, int32_t id)
	{
		RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedServiceInfo2SubscriptionDirector>, _1, id));
		if (!events_connected)
		{
			events_connected.data() = true;
			RR_WEAK_PTR<WrappedServiceInfo2Subscription> weak_this = shared_from_this();
			subscription->AddServiceDetectedListener(boost::bind(&WrappedServiceInfo2Subscription::ServiceDetected, weak_this, _1, _2, _3));
			subscription->AddServiceLostListener(boost::bind(&WrappedServiceInfo2Subscription::ServiceLost, weak_this, _1, _2, _3));
		}
	}

	void WrappedServiceInfo2Subscription::ServiceDetected(RR_WEAK_PTR<WrappedServiceInfo2Subscription> this_, RR_SHARED_PTR<ServiceInfo2Subscription> subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info)
	{
		RR_SHARED_PTR<WrappedServiceInfo2Subscription> this1 = this_.lock();
		if (!this1) return;
		this1->ServiceDetected1(subscription, id, info);
	}

	void WrappedServiceInfo2Subscription::ServiceLost(RR_WEAK_PTR<WrappedServiceInfo2Subscription> this_, RR_SHARED_PTR<ServiceInfo2Subscription> subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info)
	{
		RR_SHARED_PTR<WrappedServiceInfo2Subscription> this1 = this_.lock();
		if (!this1) return;
		this1->ServiceLost1(subscription, id, info);
	}

	void WrappedServiceInfo2Subscription::ServiceDetected1(RR_SHARED_PTR<ServiceInfo2Subscription>& subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info)
	{
		RR_SHARED_PTR<WrappedServiceInfo2Subscription> s = shared_from_this();
		ServiceInfo2Wrapped info2(info);
		DIRECTOR_CALL3(WrappedServiceInfo2SubscriptionDirector, RR_Director->ServiceDetected(s, id, info2));
	}

	void WrappedServiceInfo2Subscription::ServiceLost1(RR_SHARED_PTR<ServiceInfo2Subscription>& subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info)
	{
		RR_SHARED_PTR<WrappedServiceInfo2Subscription> s = shared_from_this();
		ServiceInfo2Wrapped info2(info);
		DIRECTOR_CALL3(WrappedServiceInfo2SubscriptionDirector, RR_Director->ServiceLost(s, id, info2));
	}
	
	WrappedServiceSubscription::WrappedServiceSubscription(RR_SHARED_PTR<ServiceSubscription> subscription)
	{
		this->subscription = subscription;
	}

	std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<WrappedServiceStub> > WrappedServiceSubscription::GetConnectedClients()
	{
		std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<WrappedServiceStub> > o;
		typedef std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<RRObject> >::value_type e_type;
		std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<RRObject> > a = subscription->GetConnectedClients();
		BOOST_FOREACH(e_type& e, a)
		{
			RR_SHARED_PTR<WrappedServiceStub> e1 = RR_DYNAMIC_POINTER_CAST<WrappedServiceStub>(e.second);

			if (!e1) continue;
			o.insert(std::make_pair(e.first, e1));			
		}

		return o;
	}

	void WrappedServiceSubscription::Close()
	{
		{
			RR_Director.reset();
		}

		subscription->Close();
	}

	void WrappedServiceSubscription::ClaimClient(RR_SHARED_PTR<WrappedServiceStub> client)
	{
		subscription->ClaimClient(client);
	}

	void WrappedServiceSubscription::ReleaseClient(RR_SHARED_PTR<WrappedServiceStub> client)
	{
		subscription->ReleaseClient(client);
	}

	uint32_t WrappedServiceSubscription::GetConnectRetryDelay()
	{
		return subscription->GetConnectRetryDelay();
	}

	void WrappedServiceSubscription::SetConnectRetryDelay(uint32_t delay_milliseconds)
	{
		subscription->SetConnectRetryDelay(delay_milliseconds);
	}

	RR_SHARED_PTR<WrappedWireSubscription> WrappedServiceSubscription::SubscribeWire(const std::string& membername)
	{
		RR_SHARED_PTR<WrappedWireSubscription> o = RR_MAKE_SHARED<WrappedWireSubscription>(subscription, membername);
		detail::ServiceSubscription_custom_member_subscribers::SubscribeWire(subscription, o);
		return o;
	}

	RR_SHARED_PTR<WrappedPipeSubscription> WrappedServiceSubscription::SubscribePipe(const std::string& membername, uint32_t max_recv_packets)
	{
		RR_SHARED_PTR<WrappedPipeSubscription> o = RR_MAKE_SHARED<WrappedPipeSubscription>(subscription, membername, max_recv_packets);
		detail::ServiceSubscription_custom_member_subscribers::SubscribePipe(subscription, o);
		return o;
	}

	void WrappedServiceSubscription::SetRRDirector(WrappedServiceSubscriptionDirector* director, int32_t id)
	{
		RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedServiceSubscriptionDirector>, _1, id));
		if (!events_connected)
		{
			events_connected.data() = true;
			RR_WEAK_PTR<WrappedServiceSubscription> weak_this=shared_from_this();
			subscription->AddClientConnectListener(boost::bind(&WrappedServiceSubscription::ClientConnected, weak_this, _1, _2, _3));
			subscription->AddClientDisconnectListener(boost::bind(&WrappedServiceSubscription::ClientDisconnected, weak_this, _1, _2, _3));
		}
	}

	void WrappedServiceSubscription::ClientConnected(RR_WEAK_PTR<WrappedServiceSubscription> this_, RR_SHARED_PTR<ServiceSubscription> subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject> client)
	{
		RR_SHARED_PTR<WrappedServiceSubscription> this1 = this_.lock();
		if (!this1) return;
		this1->ClientConnected1(subscription, id, client);
	}
	void WrappedServiceSubscription::ClientDisconnected(RR_WEAK_PTR<WrappedServiceSubscription> this_, RR_SHARED_PTR<ServiceSubscription> subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject> client)
	{
		RR_SHARED_PTR<WrappedServiceSubscription> this1 = this_.lock();
		if (!this1) return;
		this1->ClientDisconnected1(subscription, id, client);
	}

	void WrappedServiceSubscription::ClientConnected1(RR_SHARED_PTR<ServiceSubscription>& subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject>& client)
	{
		RR_SHARED_PTR<WrappedServiceSubscription> s = shared_from_this();
		RR_SHARED_PTR<WrappedServiceStub> client2 = RR_DYNAMIC_POINTER_CAST<WrappedServiceStub>(client);
		DIRECTOR_CALL3(WrappedServiceSubscriptionDirector, RR_Director->ClientConnected(s, id, client2));
	}
	void WrappedServiceSubscription::ClientDisconnected1(RR_SHARED_PTR<ServiceSubscription>& subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject>& client)
	{
		RR_SHARED_PTR<WrappedServiceSubscription> s = shared_from_this();
		RR_SHARED_PTR<WrappedServiceStub> client2 = RR_DYNAMIC_POINTER_CAST<WrappedServiceStub>(client);
		DIRECTOR_CALL3(WrappedServiceSubscriptionDirector, RR_Director->ClientDisconnected(s, id, client2));
	}

	WrappedWireSubscription::WrappedWireSubscription(RR_SHARED_PTR<ServiceSubscription> parent, const std::string& membername)
		: WireSubscriptionBase(parent, membername)
	{
		
	}

	WrappedService_typed_packet WrappedWireSubscription::GetInValue(TimeSpec* time)
	{
		RR_SHARED_PTR<WireConnectionBase> connection1;

		WrappedService_typed_packet o;

		o.packet = RR_STATIC_POINTER_CAST<MessageElement>(GetInValueBase(time, &connection1));
		if (!connection1) throw InvalidOperationException("Invalid subscription wire client");
		RR_SHARED_PTR<WrappedWireConnection> connection2 = rr_cast<WrappedWireConnection>(connection1);
		o.type = connection2->Type;
		o.stub = connection2->GetStub();
		return o;		
	}

	bool WrappedWireSubscription::TryGetInValue(WrappedService_typed_packet& val, TimeSpec* time)
	{
		RR_SHARED_PTR<WireConnectionBase> connection1;		

		RR_INTRUSIVE_PTR<RRValue> packet1;
		bool ret = TryGetInValueBase(packet1, time, &connection1);
		if (!ret) return false;
		val.packet = RR_STATIC_POINTER_CAST<MessageElement>(packet1);
		if (!connection1) throw InvalidOperationException("Invalid subscription wire client");
		RR_SHARED_PTR<WrappedWireConnection> connection2 = rr_cast<WrappedWireConnection>(connection1);
		val.type = connection2->Type;
		val.stub = connection2->GetStub();
		return ret;
	}
		
	void WrappedWireSubscription::SetRRDirector(WrappedWireSubscriptionDirector* director, int32_t id)
	{
		RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedWireSubscriptionDirector>, _1, id));		
	}

	
	void WrappedWireSubscription::fire_WireValueChanged(RR_INTRUSIVE_PTR<RRValue> value, const TimeSpec& time, RR_SHARED_PTR<WireConnectionBase> connection)
	{
		WrappedService_typed_packet val;
		val.packet = RR_STATIC_POINTER_CAST<MessageElement>(value);
		RR_SHARED_PTR<WrappedWireConnection> connection2 = rr_cast<WrappedWireConnection>(connection);
		val.type = connection2->Type;
		val.stub = connection2->GetStub();
		RR_SHARED_PTR<WrappedWireSubscription> s = RR_STATIC_POINTER_CAST<WrappedWireSubscription>(shared_from_this());
		DIRECTOR_CALL3(WrappedWireSubscriptionDirector, RR_Director->WireValueChanged(s, val, time));
	}

	WrappedWireSubscription_send_iterator::WrappedWireSubscription_send_iterator(const RR_SHARED_PTR<WrappedWireSubscription>& sub)
		: iter(sub)
	{

	}

	RR_SHARED_PTR<WrappedWireConnection> WrappedWireSubscription_send_iterator::Next()
	{
		current_connection = RR_DYNAMIC_POINTER_CAST<WrappedWireConnection>(iter.Next());
		return current_connection;
	}

	RR_SHARED_PTR<WrappedServiceStub> WrappedWireSubscription_send_iterator::GetStub()
	{
		if (!current_connection)
		{
			return RR_SHARED_PTR<WrappedServiceStub>();
		}
		return current_connection->GetStub();
	}

	void WrappedWireSubscription_send_iterator::SetOutValue(const RR_INTRUSIVE_PTR<MessageElement>& value)
	{
		iter.SetOutValue(value);
	}

	RR_SHARED_PTR<TypeDefinition> WrappedWireSubscription_send_iterator::GetType()
	{
		if (!current_connection)
		{
			throw InvalidOperationException("Invalid operation");
		}

		return current_connection->Type;
	}

	WrappedWireSubscription_send_iterator::~WrappedWireSubscription_send_iterator()
	{

	}


	WrappedPipeSubscription::WrappedPipeSubscription(RR_SHARED_PTR<ServiceSubscription> parent, const std::string& membername, int32_t max_recv_packets, int32_t max_send_backlog)
		: PipeSubscriptionBase(parent, membername, max_recv_packets, max_send_backlog)
	{
		
	}

	WrappedService_typed_packet WrappedPipeSubscription::ReceivePacket()
	{
		WrappedService_typed_packet o;
		bool ret = TryReceivePacket(o);
		if (!ret)
		{
			throw InvalidOperationException("PipeSubscription Receive Queue Empty");
		}
		return o;
	}
	
	bool WrappedPipeSubscription::TryReceivePacket(WrappedService_typed_packet& packet, bool peek)
	{
		RR_SHARED_PTR<PipeEndpointBase> endpoint1;
		RR_INTRUSIVE_PTR<RRValue> packet1;
		bool ret = PipeSubscriptionBase::TryReceivePacketBase(packet1, peek, &endpoint1);
		if (!ret) return false;
		packet.packet = RR_STATIC_POINTER_CAST<MessageElement>(packet.packet);
		if (!endpoint1) throw InvalidOperationException("Invalid subscription pipe endpoint");
		RR_SHARED_PTR<WrappedPipeEndpoint> endpoint2 = rr_cast<WrappedPipeEndpoint>(endpoint1);
		packet.type = endpoint2->Type;
		packet.stub = endpoint2->GetStub();
		return ret;
	}
		
	void WrappedPipeSubscription::SetRRDirector(WrappedPipeSubscriptionDirector* director, int32_t id)
	{
		RR_Director.reset(director, boost::bind(&ReleaseDirector<WrappedPipeSubscriptionDirector>, _1, id));		
	}

	
	void WrappedPipeSubscription::fire_PipePacketReceived()
	{
		RR_SHARED_PTR<WrappedPipeSubscription> s = RR_STATIC_POINTER_CAST<WrappedPipeSubscription>(shared_from_this());
		DIRECTOR_CALL3(WrappedPipeSubscriptionDirector, RR_Director->PipePacketReceived(s));
	}

	WrappedPipeSubscription_send_iterator::WrappedPipeSubscription_send_iterator(const RR_SHARED_PTR<WrappedPipeSubscription>& sub)
		: iter(sub)
	{

	}

	RR_SHARED_PTR<WrappedPipeEndpoint> WrappedPipeSubscription_send_iterator::Next()
	{
		current_connection = RR_DYNAMIC_POINTER_CAST<WrappedPipeEndpoint>(iter.Next());
		return current_connection;
	}

	void WrappedPipeSubscription_send_iterator::AsyncSendPacket(const RR_INTRUSIVE_PTR<MessageElement>& value)
	{
		iter.AsyncSendPacket(value);
	}

	RR_SHARED_PTR<TypeDefinition> WrappedPipeSubscription_send_iterator::GetType()
	{
		if (!current_connection)
		{
			throw InvalidOperationException("Invalid operation");
		}
		return current_connection->Type;
	}

	RR_SHARED_PTR<WrappedServiceStub> WrappedPipeSubscription_send_iterator::GetStub()
	{

		if (!current_connection)
		{
			return RR_SHARED_PTR<WrappedServiceStub>();
		}
		return current_connection->GetStub();
	}

	WrappedPipeSubscription_send_iterator::~WrappedPipeSubscription_send_iterator()
	{

	}

	std::vector<ServiceSubscriptionClientID> WrappedServiceInfo2SubscriptionServicesToVector(std::map<ServiceSubscriptionClientID, ServiceInfo2Wrapped >& infos)
	{
		std::vector<ServiceSubscriptionClientID> o;
		BOOST_FOREACH(const ServiceSubscriptionClientID& id, infos | boost::adaptors::map_keys)
		{
			o.push_back(id);
		}
		return o;
	}

	std::vector<ServiceSubscriptionClientID> WrappedServiceSubscriptionClientsToVector(std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<WrappedServiceStub> >& clients)
	{
		std::vector<ServiceSubscriptionClientID> o;
		BOOST_FOREACH(const ServiceSubscriptionClientID& id, clients | boost::adaptors::map_keys)
		{
			o.push_back(id);
		}
		return o;
	}

	static RR_SHARED_PTR<ServiceSubscriptionFilter> WrappedSubscribeService_LoadFilter(RR_SHARED_PTR<RobotRaconteurNode>& node, RR_SHARED_PTR<WrappedServiceSubscriptionFilter>& filter)
	{
		RR_SHARED_PTR<ServiceSubscriptionFilter> filter2;
		if (filter)
		{
			filter2 = RR_MAKE_SHARED<ServiceSubscriptionFilter>();
			filter2->ServiceNames = filter->ServiceNames;
			filter2->TransportSchemes = filter->TransportSchemes;
			filter2->MaxConnections = filter->MaxConnections;
			BOOST_FOREACH(RR_SHARED_PTR<WrappedServiceSubscriptionFilterNode>& n, filter->Nodes)
			{
				if (!n) continue;
				RR_SHARED_PTR<ServiceSubscriptionFilterNode> n2 = RR_MAKE_SHARED<ServiceSubscriptionFilterNode>();
				n2->NodeID = n->NodeID;
				n2->NodeName = n->NodeName;
				n2->Username = n->Username;
				n2->Credentials = node->UnpackMapType<std::string, RRValue>(RR_DYNAMIC_POINTER_CAST<MessageElementMap<std::string> >(n->Credentials));
				filter2->Nodes.push_back(n2);
			}

			if (filter->Predicate)
			{
				filter2->Predicate = boost::bind(&WrappedServiceSubscriptionFilterPredicateDirector::CallPredicate, filter->Predicate, _1);
			}
		}

		return filter2;
	}

	RR_SHARED_PTR<WrappedServiceInfo2Subscription> WrappedSubscribeServiceInfo2(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& service_types, RR_SHARED_PTR<WrappedServiceSubscriptionFilter> filter)
	{
		RR_SHARED_PTR<ServiceSubscriptionFilter> filter2 = WrappedSubscribeService_LoadFilter(node, filter);

		RR_SHARED_PTR<ServiceInfo2Subscription> sub = node->SubscribeServiceInfo2(service_types, filter2);

		return RR_MAKE_SHARED<WrappedServiceInfo2Subscription>(sub);
	}

	RR_SHARED_PTR<WrappedServiceSubscription> WrappedSubscribeService(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& service_types, RR_SHARED_PTR<WrappedServiceSubscriptionFilter> filter)
	{
		
		RR_SHARED_PTR<ServiceSubscriptionFilter> filter2=WrappedSubscribeService_LoadFilter(node, filter);

		RR_SHARED_PTR<ServiceSubscription> sub = node->SubscribeService(service_types, filter2);

		return RR_MAKE_SHARED<WrappedServiceSubscription>(sub);
	}

}
