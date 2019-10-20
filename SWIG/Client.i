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

%shared_ptr(RobotRaconteur::WrappedServiceStub)
%shared_ptr(RobotRaconteur::RobotRaconteurNode)

%feature("director") RobotRaconteur::WrappedServiceStubDirector;
%feature("director") RobotRaconteur::AsyncStubReturnDirector;

//ServiceStub

namespace RobotRaconteur
{

class RobotRaconteurNode;

RR_DIRECTOR_SHARED_PTR_RETURN_RESET(RobotRaconteur::MessageElement)
class WrappedServiceStubDirector
{
public:
	virtual ~WrappedServiceStubDirector() {}
	virtual void DispatchEvent(const std::string& EventName, std::vector<boost::intrusive_ptr<RobotRaconteur::MessageElement> > args);
	virtual boost::intrusive_ptr<RobotRaconteur::MessageElement> CallbackCall(const std::string& CallbackName, std::vector<boost::intrusive_ptr<RobotRaconteur::MessageElement> > args);
};
RR_DIRECTOR_SHARED_PTR_RETURN_DEFAULT(RobotRaconteur::MessageElement)

class AsyncStubReturnDirector
{
public:
	virtual ~AsyncStubReturnDirector() {}
	virtual void handler(boost::shared_ptr<RobotRaconteur::WrappedServiceStub> stub, uint32_t error_code, const std::string& errorname, const std::string& errormessage);
};

%nodefaultctor WrappedServiceStub;
class WrappedServiceStub : public virtual RobotRaconteur::RRObject
{
public:

	virtual void async_PropertyGet(const std::string& PropertyName, int32_t timeout, AsyncRequestDirector* handler,int32_t id);
	virtual void async_PropertySet(const std::string& PropertyName, boost::intrusive_ptr<RobotRaconteur::MessageElement> value, int32_t timeout, AsyncRequestDirector* handler,int32_t id);
	virtual void async_FunctionCall(const std::string& FunctionName, const std::vector<boost::intrusive_ptr<RobotRaconteur::MessageElement> >& args, int32_t timeout, AsyncRequestDirector* handler, int32_t id);
	virtual void async_GeneratorFunctionCall(const std::string& FunctionName, const std::vector<boost::intrusive_ptr<RobotRaconteur::MessageElement> >& args, int32_t timeout, AsyncGeneratorClientReturnDirector* handler, int32_t id);
	
	virtual void async_FindObjRef(const std::string& path, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
	virtual void async_FindObjRef(const std::string& path, const std::string& ind, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
	virtual void async_FindObjRefTyped(const std::string& path, const std::string& type, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
	virtual void async_FindObjRefTyped(const std::string& path, const std::string& ind, const std::string& type, int32_t timeout, AsyncStubReturnDirector* handler, int32_t id);
		
	virtual boost::shared_ptr<RobotRaconteur::WrappedPipeClient> GetPipe(const std::string& membername);
	virtual boost::shared_ptr<RobotRaconteur::WrappedWireClient> GetWire(const std::string& membername);
	int GetObjectHeapID();

	virtual void RRClose();
	
	boost::shared_ptr<RobotRaconteur::ServiceEntryDefinition> RR_objecttype;
	virtual std::string RRType();
	//WrappedServiceStubDirector* RR_Director;
		
	void SetRRDirector(WrappedServiceStubDirector* director, int32_t id);
	
	boost::shared_ptr<RobotRaconteur::RobotRaconteurNode> RRGetNode();
	
#ifdef SWIGPYTHON


	PyObject* GetPyStub();	
	void SetPyStub(PyObject* stub);

	
#endif
	
};

}
