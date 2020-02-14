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

#include <RobotRaconteur.h>

#ifdef RR_PYTHON
#include <Python.h>
#endif
#pragma once



namespace RobotRaconteur
{
#ifdef RR_PYTHON

	class RR_Release_GIL
	{
	public:
		RR_Release_GIL()
		{
			_save = PyEval_SaveThread();
		}

		~RR_Release_GIL()
		{
			PyEval_RestoreThread(_save);
		}

	protected:
		PyThreadState * _save;
	};

	class RR_Ensure_GIL
	{
	public:
		RR_Ensure_GIL()
		{
			gstate = PyGILState_Ensure();
		}

		~RR_Ensure_GIL()
		{
			PyGILState_Release(gstate);
		}
	private:
		PyGILState_STATE gstate;

	};

#define DIRECTOR_CALL(dirtype,command){ \
	RR_Ensure_GIL gil; \
	boost::shared_ptr<dirtype> RR_Director2(this->RR_Director); \
	\
	 \
	if (!RR_Director2) throw InvalidOperationException("Director has been released");\
	{command;} \
}

#define DIRECTOR_CALL2(command) { \
RR_Ensure_GIL gil; \
{command ;}\
}

#define DIRECTOR_CALL3(dirtype,command){ \
	RR_Ensure_GIL gil; \
	boost::shared_ptr<dirtype> RR_Director2(this->RR_Director); \
	\
	 \
	if (RR_Director2)\
	{command; } \
}



#define DIRECTOR_DELETE(var) { if(var) {delete var; var=NULL;}}

#else
#define DIRECTOR_CALL(dirtype,command) { \
\
	boost::shared_ptr<dirtype> RR_Director2(this->RR_Director); \
	\
	\
	if (!RR_Director2) throw InvalidOperationException("Director has been released");\
	RRDirectorExceptionHelper::Reset(); \
	{command;} \
	if (RobotRaconteur::RRDirectorExceptionHelper::IsErrorPending()) RobotRaconteurExceptionUtil::ThrowMessageEntryException(RobotRaconteur::RRDirectorExceptionHelper::GetError()); }

#define DIRECTOR_CALL2(command) \
	RRDirectorExceptionHelper::Reset(); \
	{command;} \
	if (RobotRaconteur::RRDirectorExceptionHelper::IsErrorPending()) RobotRaconteurExceptionUtil::ThrowMessageEntryException(RobotRaconteur::RRDirectorExceptionHelper::GetError());

#define DIRECTOR_CALL3(dirtype,command) { \
    \
	boost::shared_ptr<dirtype> RR_Director2(this->RR_Director); \
	\
	 \
	if (!RR_Director2) return;\
	RRDirectorExceptionHelper::Reset(); \
	{command;} \
	if (RobotRaconteur::RRDirectorExceptionHelper::IsErrorPending()) RobotRaconteurExceptionUtil::ThrowMessageEntryException(RobotRaconteur::RRDirectorExceptionHelper::GetError()); }

#define DIRECTOR_DELETE(var)
#endif
	
	

	

	class RRDirectorExceptionHelper
	{
	protected:
		static RR_INTRUSIVE_PTR<MessageEntry> last_err;

	public:
		static void Reset();
		static void SetError(RR_INTRUSIVE_PTR<MessageEntry> err);
		static bool IsErrorPending();
		static RR_INTRUSIVE_PTR<MessageEntry> GetError();

	};


#ifdef RR_PYTHON
	class RRNativeDirectorSupport
	{
	protected:
		static bool running;
		
	public:
		static void Start()
		{
			running = true;
		}
		static void Stop()
		{
			running = false;
		}
		static bool IsRunning()
		{
			return running;
		}

	};
	
#endif

	template<typename T>
	void ReleaseDirector(T* ptr, int32_t id)
	{
#ifdef RR_PYTHON
		if (RRNativeDirectorSupport::IsRunning())
		{
			DIRECTOR_CALL2(DIRECTOR_DELETE(ptr));
		}
#else
		DIRECTOR_CALL2(DIRECTOR_DELETE(ptr));
#endif

		
	}
	



	class WrappedServiceFactory : public virtual RobotRaconteur::ServiceFactory
	{
	public:

		WrappedServiceFactory(const std::string& defstring);
		WrappedServiceFactory(boost::shared_ptr<RobotRaconteur::ServiceDefinition> def);
		virtual ~WrappedServiceFactory() {}

		virtual std::string GetServiceName();
		virtual std::string DefString();
		virtual RR_SHARED_PTR<ServiceDefinition> ServiceDef();
		virtual RR_SHARED_PTR<RobotRaconteur::StructureStub> FindStructureStub(boost::string_ref s);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> PackStructure(RR_INTRUSIVE_PTR<RobotRaconteur::RRStructure> structin);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::RRValue> UnpackStructure(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> mstructin);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> PackPodArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseArray> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseArray> UnpackPodArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> PackPodMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseMultiDimArray> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::RRPodBaseMultiDimArray> UnpackPodMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> PackNamedArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseArray> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseArray> UnpackNamedArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> PackNamedMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseMultiDimArray> structure);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::RRNamedBaseMultiDimArray> UnpackNamedMultiDimArray(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElementNestedElementList> structure);
		virtual RR_SHARED_PTR<RobotRaconteur::ServiceStub> CreateStub(boost::string_ref objecttype, boost::string_ref path, RR_SHARED_PTR<RobotRaconteur::ClientContext> context);
		
		virtual void DownCastAndThrowException(RobotRaconteurException& exp)
		{
			throw exp;
		}

		virtual RR_SHARED_PTR<RobotRaconteurException> DownCastException(RR_SHARED_PTR<RobotRaconteurException> exp)
		{
			return exp;
		}

	private:
		RR_SHARED_PTR<ServiceDefinition> servicedef;
		std::string defstring;
	};

	class WrappedDynamicServiceFactory : public virtual DynamicServiceFactory
	{
	public:
		virtual ~WrappedDynamicServiceFactory() {}
		virtual RR_SHARED_PTR<ServiceFactory> CreateServiceFactory(boost::string_ref def);
		virtual std::vector<RR_SHARED_PTR<ServiceFactory> > CreateServiceFactories(const std::vector<std::string>& def);
	};

	
	/*class AsyncHandlerDirector
	{
	public:
		virtual ~AsyncHandlerDirector() {}
		virtual void handler(void* m, uint32_t error_code, std::string errorname, std::string errormessage) {};

	};*/

	class HandlerErrorInfo
	{
	public:
		uint32_t error_code;
		std::string errorname;
		std::string errormessage;
		std::string errorsubname;
		boost::intrusive_ptr<RobotRaconteur::MessageElement> param_;

		HandlerErrorInfo();
		HandlerErrorInfo(const RobotRaconteurException& exp);
		HandlerErrorInfo(boost::shared_ptr<RobotRaconteurException> exp);
		HandlerErrorInfo(boost::intrusive_ptr<MessageEntry> m);
	};

	class AsyncRequestDirector
	{
	public:
		virtual ~AsyncRequestDirector() {}
		virtual void handler(RR_INTRUSIVE_PTR<MessageElement> ret, HandlerErrorInfo& error) {};

	};

	class AsyncVoidReturnDirector
	{
	public:
		virtual ~AsyncVoidReturnDirector() {}
		virtual void handler(HandlerErrorInfo& error) {};
	};

	class AsyncVoidNoErrReturnDirector
	{
	public:
		virtual ~AsyncVoidNoErrReturnDirector() {}
		virtual void handler() {};
	};

	class AsyncStringReturnDirector
	{
	public:
		virtual ~AsyncStringReturnDirector() {}
		virtual void handler(const std::string& ret, HandlerErrorInfo& error) {};
	};

	class AsyncUInt32ReturnDirector
	{
	public:
		virtual ~AsyncUInt32ReturnDirector() {}
		virtual void handler(uint32_t ret, HandlerErrorInfo& error) {};
	};

	class AsyncTimerEventReturnDirector
	{
	public:
		virtual ~AsyncTimerEventReturnDirector() {}
		virtual void handler(const TimerEvent& ret, HandlerErrorInfo& error) {};
	};

	
	class WrappedServiceStubDirector
	{
	public:
		virtual ~WrappedServiceStubDirector() {}
		virtual void DispatchEvent(const std::string& EventName, const std::vector<RR_INTRUSIVE_PTR<MessageElement> > args) {}
		virtual RR_INTRUSIVE_PTR<MessageElement> CallbackCall(const std::string& CallbackName, const std::vector<RR_INTRUSIVE_PTR<MessageElement> > args) { return RR_INTRUSIVE_PTR<MessageElement>(); }

	};

	class WrappedPipeClient;
	class WrappedWireClient;
	class WrappedServiceStub;
	class WrappedGeneratorClient;
	class AsyncGeneratorClientReturnDirector;
	
	class AsyncStubReturnDirector
	{
	public:
		virtual ~AsyncStubReturnDirector() {}
		virtual void handler(boost::shared_ptr<WrappedServiceStub> stub, HandlerErrorInfo& error) {};
	};

	class WrappedServiceStub : public virtual RobotRaconteur::ServiceStub
	{
	public:
		WrappedServiceStub(boost::string_ref path, RR_SHARED_PTR<ServiceEntryDefinition> type, RR_SHARED_PTR<RobotRaconteur::ClientContext> c);
		virtual ~WrappedServiceStub();

		virtual void async_PropertyGet(const std::string& PropertyName, int32_t timeout, AsyncRequestDirector* handler,int32_t id);
		virtual void async_PropertySet(const std::string& PropertyName, RR_INTRUSIVE_PTR<MessageElement> value, int32_t timeout, AsyncRequestDirector* handler,int32_t id);
		virtual void async_FunctionCall(const std::string& FunctionName, const std::vector<RR_INTRUSIVE_PTR<MessageElement> >& args, int32_t timeout, AsyncRequestDirector* handler, int32_t id);
		virtual void async_GeneratorFunctionCall(const std::string& FunctionName, const std::vector<RR_INTRUSIVE_PTR<MessageElement> >& args, int32_t timeout, AsyncGeneratorClientReturnDirector* handler, int32_t id);
		virtual void async_FindObjRef(const std::string& path, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
		virtual void async_FindObjRef(const std::string& path, const std::string& ind, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
		virtual void async_FindObjRefTyped(const std::string& path, const std::string& type, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
		virtual void async_FindObjRefTyped(const std::string& path, const std::string& ind, const std::string& type, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
		
		
	protected:
		virtual void async_PropertyGet_handler( RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler);
		virtual void async_PropertySet_handler( RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler);
		virtual void async_FunctionCall_handler( RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler);
		virtual void async_GeneratorFunctionCall_handler(const std::string& FunctionName, RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncGeneratorClientReturnDirector> handler);
		virtual void async_FindObjRef_handler( RR_SHARED_PTR<RRObject> stub, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncStubReturnDirector> handler);
	public:

		virtual void DispatchEvent(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m);
		virtual void DispatchPipeMessage(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m);
		virtual void DispatchWireMessage(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m);
		virtual RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> CallbackCall(RR_INTRUSIVE_PTR<RobotRaconteur::MessageEntry> m);
		virtual RR_SHARED_PTR<RobotRaconteur::WrappedPipeClient> GetPipe(const std::string& membername);
		virtual RR_SHARED_PTR<RobotRaconteur::WrappedWireClient> GetWire(const std::string& membername);
		virtual RR_SHARED_PTR<PipeClientBase> RRGetPipeClient(boost::string_ref membername);
		virtual RR_SHARED_PTR<WireClientBase> RRGetWireClient(boost::string_ref membername);
		
		virtual void RRClose();
		virtual void RRInitStub();
	public:
		RR_SHARED_PTR<ServiceEntryDefinition> RR_objecttype;
		
	public:
		virtual std::string RRType();
	protected:
		boost::shared_ptr<WrappedServiceStubDirector> RR_Director;

		int objectheapid;
	public:

		//WrappedServiceStubDirector* GetRRDirector();
		void SetRRDirector(WrappedServiceStubDirector* director, int32_t id);

		std::map<std::string, RR_SHARED_PTR<WrappedPipeClient> > pipes;
		std::map<std::string, RR_SHARED_PTR<WrappedWireClient> > wires;
		
		int GetObjectHeapID();

		//int32_t objectheapid;

#ifdef RR_PYTHON
	protected:
		PyObject* pystub;

	public:

		PyObject* GetPyStub()
		{
            if (pystub!=NULL)
            {
                Py_XINCREF(pystub);
                return pystub;
            }
            else
            {
                PyObject* n=Py_None;
                Py_XINCREF(n);
                return n;
            }
		}

		void SetPyStub(PyObject* stub)
		{
            if (pystub!=NULL)
            {
			Py_XDECREF(pystub);
            }
            if (stub==Py_None)
            {
                pystub=NULL;
            }
            else
            {
                Py_XINCREF(stub);
                pystub=stub;
            }
		}

#endif

	};

	class WrappedStubCallbackDirector
	{
	public:
		virtual ~WrappedStubCallbackDirector() {}
		virtual void Callback(ClientServiceListenerEventType) {}
	};

	class WrappedPipeEndpoint;
	class WrappedPipeEndpointDirector
	{
	public:
		virtual ~WrappedPipeEndpointDirector() {}
		virtual void PipeEndpointClosedCallback() {};
		virtual void PacketReceivedEvent() {};
		virtual void PacketAckReceivedEvent(uint32_t packetnum) {};

	};

	class WrappedTryReceivePacketWaitResult
	{
	public:
		bool res;
		RR_INTRUSIVE_PTR<MessageElement> packet;
	};

	class WrappedPipeEndpoint : public PipeEndpointBase
	{

	public:
		
		friend class WrappedPipeBroadcaster;

		virtual ~WrappedPipeEndpoint();

		
		virtual void AsyncSendPacket(RR_INTRUSIVE_PTR<MessageElement> packet, AsyncUInt32ReturnDirector* handler, int32_t id);
		virtual RR_INTRUSIVE_PTR<MessageElement> ReceivePacket();
		virtual RR_INTRUSIVE_PTR<MessageElement> PeekNextPacket();
		
		virtual WrappedTryReceivePacketWaitResult TryReceivePacketWait(int32_t timeout = RR_TIMEOUT_INFINITE, bool peek = false);
		
		WrappedPipeEndpoint(RR_SHARED_PTR<PipeBase> parent, int32_t index, uint32_t endpoint, RR_SHARED_PTR<TypeDefinition> Type, bool unreliable, MemberDefinition_Direction direction, bool message3);
		RR_SHARED_PTR<TypeDefinition> Type;

	protected:
		virtual void fire_PipeEndpointClosedCallback();
		virtual void fire_PacketReceivedEvent();
		virtual void fire_PacketAckReceivedEvent(uint32_t packetnum);
	

	
		boost::shared_ptr<WrappedPipeEndpointDirector> RR_Director;


		static void send_handler(uint32_t packetnumber, RR_SHARED_PTR<RobotRaconteurException> err, boost::function<void(RR_SHARED_PTR<uint32_t>,RR_SHARED_PTR<RobotRaconteurException>)> handler)
		{
			handler(RR_MAKE_SHARED<uint32_t>(packetnumber),err);
		}

		void AsyncSendPacket_handler(uint32_t id, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncUInt32ReturnDirector> handler);


	public:

		//WrappedPipeEndpointDirector* GetRRDirector();
		void SetRRDirector(WrappedPipeEndpointDirector* director, int32_t id);
		RR_SHARED_PTR<WrappedServiceStub> GetStub();

		//int32_t objectheapid;

		using PipeEndpointBase::AsyncClose;
		void AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);
	protected:
		void AsyncClose_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler);
	};

	class AsyncPipeEndpointReturnDirector
	{
	public:
		virtual ~AsyncPipeEndpointReturnDirector() {}
		virtual void handler(boost::shared_ptr<WrappedPipeEndpoint> ep, HandlerErrorInfo& error) {};
	};

	class WrappedPipeClient : public virtual PipeClientBase
	{
	public:

		virtual ~WrappedPipeClient() {}
		
		//virtual boost::function<void(RR_SHARED_PTR<WrappedPipeEndpoint>)> GetPipeConnectCallback();
		//virtual void SetPipeConnectCallback(boost::function<void(RR_SHARED_PTR<WrappedPipeEndpoint>)> function);
		
		void AsyncConnect(int32_t index, int32_t timeout, AsyncPipeEndpointReturnDirector* handler, int32_t id);
		WrappedPipeClient(const std::string& name, RR_SHARED_PTR<ServiceStub> stub, RR_SHARED_PTR<TypeDefinition> Type, bool unreliable, MemberDefinition_Direction direction);

		
		RR_SHARED_PTR<TypeDefinition> Type;

	protected:
		virtual RR_SHARED_PTR<PipeEndpointBase> CreateNewPipeEndpoint(int32_t index, bool unreliable, MemberDefinition_Direction direction, bool message3);
		void AsyncConnect_handler(RR_SHARED_PTR<PipeEndpointBase> ep, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncPipeEndpointReturnDirector> handler);
	};

	class WrappedWireConnection;
	class WrappedWireConnectionDirector
	{
	public:
		virtual ~WrappedWireConnectionDirector() {}
		virtual void WireValueChanged(RR_INTRUSIVE_PTR<MessageElement> value, const TimeSpec& time) {}
		virtual void WireConnectionClosedCallback() {}
	};

	class TryGetValueResult
	{
	public:
		bool res;
		RR_INTRUSIVE_PTR<MessageElement> value;
		TimeSpec ts;
	};

	class WrappedWireConnection : public virtual WireConnectionBase
	{
	public:

		friend class WrappedWireBroadcaster;
		friend class WrappedWireUnicastReceiver;

		virtual ~WrappedWireConnection();
		virtual RR_INTRUSIVE_PTR<MessageElement> GetInValue();
		virtual RR_INTRUSIVE_PTR<MessageElement> GetOutValue();
		virtual void SetOutValue(RR_INTRUSIVE_PTR<MessageElement> value);

		TryGetValueResult TryGetInValue();
		TryGetValueResult TryGetOutValue();

		WrappedWireConnection(RR_SHARED_PTR<WireBase> parent, uint32_t endpoint, RR_SHARED_PTR<TypeDefinition> Type, MemberDefinition_Direction direction, bool message3) ;

		virtual void fire_WireValueChanged(RR_INTRUSIVE_PTR<RRValue> value, TimeSpec time);
		virtual void fire_WireClosedCallback();
						
		RR_SHARED_PTR<TypeDefinition> Type;
		
	protected:

		boost::shared_ptr<WrappedWireConnectionDirector> RR_Director;

	public:
		//WrappedWireConnectionDirector* GetRRDirector();
		void SetRRDirector(WrappedWireConnectionDirector* director, int32_t id);
		RR_SHARED_PTR<WrappedServiceStub> GetStub();

		//int32_t objectheapid;
		
        using WireConnectionBase::AsyncClose;
		void AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);
	protected:
		void AsyncClose_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler);
		
	};

	class AsyncWireConnectionReturnDirector
	{
	public:
		virtual ~AsyncWireConnectionReturnDirector() {}
		virtual void handler(boost::shared_ptr<WrappedWireConnection> ep, HandlerErrorInfo& error) {};
	};

	class AsyncWirePeekReturnDirector
	{
	public:
		virtual ~AsyncWirePeekReturnDirector() {}
		virtual void handler(RR_INTRUSIVE_PTR<MessageElement> value, const TimeSpec& ts, HandlerErrorInfo& error) {};
	};

	class WrappedWireClient : public virtual WireClientBase
	{
	public:
		virtual ~WrappedWireClient() {}

		void AsyncConnect(int32_t timeout, AsyncWireConnectionReturnDirector* handler, int32_t id);
		WrappedWireClient(const std::string& name, RR_SHARED_PTR<ServiceStub> stub, RR_SHARED_PTR<TypeDefinition> Type, MemberDefinition_Direction direction);
		
		void AsyncPeekInValue(int32_t timeout, AsyncWirePeekReturnDirector* handler, int32_t id);
		void AsyncPeekOutValue(int32_t timeout, AsyncWirePeekReturnDirector* handler, int32_t id);
		void AsyncPokeOutValue(const RR_INTRUSIVE_PTR<MessageElement>& value, int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);

		void AsyncPeekValue_handler(const RR_INTRUSIVE_PTR<RRValue>& value, const TimeSpec& ts, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncWirePeekReturnDirector> handler);
		void AsyncPokeValue_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler);

		RR_SHARED_PTR<TypeDefinition> Type;

	protected:
		virtual RR_SHARED_PTR<WireConnectionBase> CreateNewWireConnection(MemberDefinition_Direction direction, bool message3);
		void AsyncConnect_handler(RR_SHARED_PTR<WireConnectionBase> ep, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncWireConnectionReturnDirector> handler);
		
	};

	class AsyncGeneratorClientReturnDirector
	{
	public:
		virtual ~AsyncGeneratorClientReturnDirector() {}
		virtual void handler(boost::shared_ptr<WrappedGeneratorClient> ret, HandlerErrorInfo& error) {};
	};

	class WrappedGeneratorClient : public GeneratorClientBase
	{
	public:
		WrappedGeneratorClient(const std::string& name, int32_t id, RR_SHARED_PTR<ServiceStub> stub);


		virtual void AsyncNext(RR_INTRUSIVE_PTR<MessageElement> v, int32_t timeout, AsyncRequestDirector* handler, int32_t id);

		virtual void AsyncAbort(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);
		virtual void AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);

	protected:

		static void AsyncNext_handler(RR_INTRUSIVE_PTR<RobotRaconteur::MessageElement> m, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncRequestDirector> handler);		
		static void AsyncAbort_handler(RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler);
	};

	class WrappedGeneratorServerDirector
	{
	public:
		WrappedGeneratorServerDirector()
		{
			objectheapid = 0;
		}
		virtual ~WrappedGeneratorServerDirector() {}

		virtual RR_INTRUSIVE_PTR<MessageElement> Next(RR_INTRUSIVE_PTR<MessageElement> m) { return RR_INTRUSIVE_PTR<MessageElement>(); }

		virtual void Abort() {}
		virtual void Close() {}

		int32_t objectheapid;
	};

	class RRMultiDimArrayUntyped
	{
	public:		
		RR_INTRUSIVE_PTR<RRBaseArray> Dims;
				
		RR_INTRUSIVE_PTR<RRBaseArray> Array;
	};

	//

	class ServiceInfo2Wrapped
	{
	public:

		std::string Name;
		std::string RootObjectType;
		std::vector<std::string> RootObjectImplements;
		std::vector<std::string> ConnectionURL;
		boost::intrusive_ptr<RobotRaconteur::MessageElement> Attributes;
		RobotRaconteur::NodeID NodeID;
		std::string NodeName;

		ServiceInfo2Wrapped() {}
		ServiceInfo2Wrapped(const ServiceInfo2& value);

	};

	std::vector<RobotRaconteur::ServiceInfo2Wrapped> WrappedFindServiceByType(RR_SHARED_PTR<RobotRaconteurNode> node, const std::string &servicetype, const std::vector<std::string>& transportschemes);

	std::vector<RobotRaconteur::NodeInfo2> WrappedFindNodeByID(RR_SHARED_PTR<RobotRaconteurNode> node, const NodeID& id, const std::vector<std::string>& transportschemes);
	
	std::vector<RobotRaconteur::NodeInfo2> WrappedFindNodeByName(RR_SHARED_PTR<RobotRaconteurNode> node, const std::string& name, const std::vector<std::string>& transportschemes);
	
	class AsyncServiceInfo2VectorReturnDirector
	{
	public:
		virtual ~AsyncServiceInfo2VectorReturnDirector() {}
		virtual void handler(const std::vector<ServiceInfo2Wrapped>& ret) {}
	};

	class AsyncNodeInfo2VectorReturnDirector
	{
	public:
		virtual ~AsyncNodeInfo2VectorReturnDirector() {}
		virtual void handler(const std::vector<NodeInfo2>& ret) {}
	};

	void AsyncServiceInfo2VectorReturn_handler(RR_SHARED_PTR<std::vector<ServiceInfo2> > ret, RR_SHARED_PTR<AsyncServiceInfo2VectorReturnDirector> handler);
	void AsyncNodeInfo2VectorReturn_handler(RR_SHARED_PTR<std::vector<NodeInfo2> > ret, RR_SHARED_PTR<AsyncNodeInfo2VectorReturnDirector> handler);

	void AsyncWrappedFindServiceByType(RR_SHARED_PTR<RobotRaconteurNode> node, const std::string &servicetype, const std::vector<std::string>& transportschemes, int32_t timeout, AsyncServiceInfo2VectorReturnDirector* handler, int32_t id1);

	void AsyncWrappedFindNodeByID(RR_SHARED_PTR<RobotRaconteurNode> node, const NodeID& id, const std::vector<std::string>& transportschemes, int32_t timeout, AsyncNodeInfo2VectorReturnDirector* handler, int32_t id1);
	
	void AsyncWrappedFindNodeByName(RR_SHARED_PTR<RobotRaconteurNode> node, const std::string& name, const std::vector<std::string>& transportschemes, int32_t timeout, AsyncNodeInfo2VectorReturnDirector* handler, int32_t id);
	
	void WrappedUpdateDetectedNodes(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& schemes);

	void AsyncWrappedUpdateDetectedNodes(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& schemes, int32_t timeout, AsyncVoidNoErrReturnDirector* handler, int32_t id1);

	std::vector<std::string> WrappedGetDetectedNodes(RR_SHARED_PTR<RobotRaconteurNode> node);

	void AsyncStubReturn_handler(RR_SHARED_PTR<RRObject> obj, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncStubReturnDirector> handler);
	
	void AsyncVoidNoErrReturn_handler(RR_SHARED_PTR<AsyncVoidNoErrReturnDirector> handler);
	

	void AsyncStringReturn_handler(RR_SHARED_PTR<std::string> str, RR_SHARED_PTR<RobotRaconteurException> err, RR_SHARED_PTR<AsyncStringReturnDirector> handler);
	
	void WrappedExceptionHandler(const std::exception* err, RR_SHARED_PTR<AsyncVoidReturnDirector> handler);

	void TimerHandlerFunc(const TimerEvent& ev, RR_SHARED_PTR<AsyncTimerEventReturnDirector> d);

	//Subscriptions

	class WrappedServiceSubscriptionFilterPredicateDirector
	{
	public:
		virtual bool Predicate(const ServiceInfo2Wrapped& info) = 0;

		virtual bool CallPredicate(const ServiceInfo2& info);

		virtual ~WrappedServiceSubscriptionFilterPredicateDirector() {}		
	};

	class WrappedServiceSubscriptionFilterNode
	{
	public:
		::RobotRaconteur::NodeID NodeID;
		std::string NodeName;
		std::string Username;
		RR_INTRUSIVE_PTR<MessageElementData> Credentials;
	};

	class WrappedServiceSubscriptionFilter
	{
	public:
		std::vector<RR_SHARED_PTR<WrappedServiceSubscriptionFilterNode> > Nodes;
		std::vector<std::string> ServiceNames;
		std::vector<std::string> TransportSchemes;
		RR_SHARED_PTR<WrappedServiceSubscriptionFilterPredicateDirector> Predicate;
		void SetRRPredicateDirector(WrappedServiceSubscriptionFilterPredicateDirector* director, int32_t id);
		uint32_t MaxConnections;
	};

	class WrappedServiceInfo2Subscription;

	class WrappedServiceInfo2SubscriptionDirector
	{
	public:
		virtual void ServiceDetected(boost::shared_ptr<WrappedServiceInfo2Subscription> subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2Wrapped& service) {}
		virtual void ServiceLost(boost::shared_ptr<WrappedServiceInfo2Subscription> subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2Wrapped& service) {}

		virtual ~WrappedServiceInfo2SubscriptionDirector() {}
	};

	class WrappedServiceInfo2Subscription : public RR_ENABLE_SHARED_FROM_THIS<WrappedServiceInfo2Subscription>
	{
	public:
		WrappedServiceInfo2Subscription(RR_SHARED_PTR<ServiceInfo2Subscription> subscription);

		std::map<ServiceSubscriptionClientID, ServiceInfo2Wrapped > GetDetectedServiceInfo2();

		void Close();

		void SetRRDirector(WrappedServiceInfo2SubscriptionDirector* director, int32_t id);

	protected:

		RR_SHARED_PTR<ServiceInfo2Subscription> subscription;
		RR_SHARED_PTR<WrappedServiceInfo2SubscriptionDirector> RR_Director;
		boost::initialized<bool> events_connected;

		static void ServiceDetected(RR_WEAK_PTR<WrappedServiceInfo2Subscription> this_, RR_SHARED_PTR<ServiceInfo2Subscription> subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info);
		static void ServiceLost(RR_WEAK_PTR<WrappedServiceInfo2Subscription> this_, RR_SHARED_PTR<ServiceInfo2Subscription> subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info);
		void ServiceDetected1(RR_SHARED_PTR<ServiceInfo2Subscription>& subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info);
		void ServiceLost1(RR_SHARED_PTR<ServiceInfo2Subscription>& subscription, const ServiceSubscriptionClientID& id, const ServiceInfo2& info);
	};


	class WrappedService_typed_packet
	{
	public:
		RR_INTRUSIVE_PTR<MessageElement> packet;
		RR_SHARED_PTR<TypeDefinition> type;
		RR_SHARED_PTR<WrappedServiceStub> stub;		
	};

	class WrappedServiceSubscription;

	class WrappedServiceSubscriptionDirector
	{
	public:
		virtual void ClientConnected(boost::shared_ptr<WrappedServiceSubscription> subscription, const ServiceSubscriptionClientID& id, boost::shared_ptr<WrappedServiceStub> client) = 0;
		virtual void ClientDisconnected(boost::shared_ptr<WrappedServiceSubscription> subscription, const ServiceSubscriptionClientID& id, boost::shared_ptr<WrappedServiceStub> client) = 0;
		
		virtual ~WrappedServiceSubscriptionDirector() {}
	};

	class WrappedWireSubscription;
	class WrappedPipeSubscription;

	class WrappedServiceSubscription : public RR_ENABLE_SHARED_FROM_THIS<WrappedServiceSubscription>
	{
	public:

		WrappedServiceSubscription(RR_SHARED_PTR<ServiceSubscription> subscription);

		std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<WrappedServiceStub> > GetConnectedClients();

		void Close();
		
		void ClaimClient(RR_SHARED_PTR<WrappedServiceStub> client);
		void ReleaseClient(RR_SHARED_PTR<WrappedServiceStub> client);

		uint32_t GetConnectRetryDelay();
		void SetConnectRetryDelay(uint32_t delay_milliseconds);

		RR_SHARED_PTR<WrappedWireSubscription> SubscribeWire(const std::string& membername);

		RR_SHARED_PTR<WrappedPipeSubscription> SubscribePipe(const std::string& membername, uint32_t max_recv_packets = std::numeric_limits<uint32_t>::max());

		void SetRRDirector(WrappedServiceSubscriptionDirector* director, int32_t id);
		
	protected:

		RR_SHARED_PTR<ServiceSubscription> subscription;
		RR_SHARED_PTR<WrappedServiceSubscriptionDirector> RR_Director;
		boost::initialized<bool> events_connected;

		static void ClientConnected(RR_WEAK_PTR<WrappedServiceSubscription> this_, RR_SHARED_PTR<ServiceSubscription> subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject> client);
		static void ClientDisconnected(RR_WEAK_PTR<WrappedServiceSubscription> this_, RR_SHARED_PTR<ServiceSubscription> subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject> client);
		void ClientConnected1(RR_SHARED_PTR<ServiceSubscription>& subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject>& client);
		void ClientDisconnected1(RR_SHARED_PTR<ServiceSubscription>& subscription, const ServiceSubscriptionClientID& id, RR_SHARED_PTR<RRObject>& client);

	};

	class WrappedWireSubscriptionDirector
	{
	public:
		virtual void WireValueChanged(boost::shared_ptr<WrappedWireSubscription> wire_subscription, WrappedService_typed_packet& value, const TimeSpec& time) = 0;
		
		virtual ~WrappedWireSubscriptionDirector() {}
	};

	class WrappedWireSubscription_send_iterator;

	class WrappedWireSubscription : public WireSubscriptionBase
	{
	public:

		friend class WrappedWireSubscription_send_iterator;

		WrappedWireSubscription(RR_SHARED_PTR<ServiceSubscription> parent, const std::string& membername);

		WrappedService_typed_packet GetInValue(TimeSpec* time = NULL);
		bool TryGetInValue(WrappedService_typed_packet& val, TimeSpec* time = NULL);
		
		void SetRRDirector(WrappedWireSubscriptionDirector* director, int32_t id);
		
	protected:

		virtual void fire_WireValueChanged(RR_INTRUSIVE_PTR<RRValue> value, const TimeSpec& time, RR_SHARED_PTR<WireConnectionBase> connection);
		RR_SHARED_PTR<WrappedWireSubscriptionDirector> RR_Director;
	};

	class WrappedWireSubscription_send_iterator
	{
	protected:
		detail::WireSubscription_send_iterator iter;
		RR_SHARED_PTR<WrappedWireConnection> current_connection;

	public:
		WrappedWireSubscription_send_iterator(const RR_SHARED_PTR<WrappedWireSubscription>& sub);
		RR_SHARED_PTR<WrappedWireConnection> Next();
		void SetOutValue(const RR_INTRUSIVE_PTR<MessageElement>& value);
		RR_SHARED_PTR<TypeDefinition> GetType();
		RR_SHARED_PTR<WrappedServiceStub> GetStub();
		virtual ~WrappedWireSubscription_send_iterator();
	};

	class WrappedPipeSubscriptionDirector
	{
	public:
		virtual void PipePacketReceived(boost::shared_ptr<WrappedPipeSubscription> pipe_subscription) = 0;

		virtual ~WrappedPipeSubscriptionDirector() {}
	};

	class WrappedPipeSubscription_send_iterator;

	class WrappedPipeSubscription : public PipeSubscriptionBase
	{
	public:

		friend class WrappedPipeSubscription_send_iterator;

		WrappedPipeSubscription(RR_SHARED_PTR<ServiceSubscription> parent, const std::string& membername, int32_t max_recv_packets = -1, int32_t max_send_backlog = 5);

		WrappedService_typed_packet ReceivePacket();
		bool TryReceivePacket(WrappedService_typed_packet& packet, bool peek = false);
		
		void SetRRDirector(WrappedPipeSubscriptionDirector* director, int32_t id);

	protected:

		virtual void fire_PipePacketReceived();
		RR_SHARED_PTR<WrappedPipeSubscriptionDirector> RR_Director;
	};

	class WrappedPipeSubscription_send_iterator
	{
	protected:
		detail::PipeSubscription_send_iterator iter;
		RR_SHARED_PTR<WrappedPipeEndpoint> current_connection;

	public:
		WrappedPipeSubscription_send_iterator(const RR_SHARED_PTR<WrappedPipeSubscription>& sub);
		RR_SHARED_PTR<WrappedPipeEndpoint> Next();
		void AsyncSendPacket(const RR_INTRUSIVE_PTR<MessageElement>& value);
		RR_SHARED_PTR<TypeDefinition> GetType();
		RR_SHARED_PTR<WrappedServiceStub> GetStub();
		virtual ~WrappedPipeSubscription_send_iterator();
	};

	std::vector<ServiceSubscriptionClientID> WrappedServiceSubscriptionClientsToVector(std::map<ServiceSubscriptionClientID, RR_SHARED_PTR<WrappedServiceStub> >& clients);

	std::vector<ServiceSubscriptionClientID> WrappedServiceInfo2SubscriptionServicesToVector(std::map<ServiceSubscriptionClientID, ServiceInfo2Wrapped>& infos);
	
	RR_SHARED_PTR<WrappedServiceInfo2Subscription> WrappedSubscribeServiceInfo2(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& service_types, RR_SHARED_PTR<WrappedServiceSubscriptionFilter> filter = RR_SHARED_PTR<WrappedServiceSubscriptionFilter>());

	RR_SHARED_PTR<WrappedServiceSubscription> WrappedSubscribeService(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<std::string>& service_types, RR_SHARED_PTR<WrappedServiceSubscriptionFilter> filter = RR_SHARED_PTR<WrappedServiceSubscriptionFilter>());
	
}
