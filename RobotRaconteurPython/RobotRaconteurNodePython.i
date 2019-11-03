

%include "RobotRaconteurNode.i"

%extend RobotRaconteur::RobotRaconteurNode
{
public:
	void Init(size_t thread_count=20)
	{
	
	$self->Init();
	
	}

%pythoncode %{
def AsyncConnectService(self, url, username, credentials, listener, handler, timeout=RR_TIMEOUT_INFINITE):
	
	from .RobotRaconteurPythonUtil import PackMessageElement, WrappedClientServiceListenerDirector, AsyncStubReturnDirectorImpl, async_call, adjust_timeout

	if (username is None ): username=""
	if (credentials is not None): credentials=PackMessageElement(credentials,"varvalue{string}",None,self).GetData()
	listener2=None
	if (listener is not None):
		listener2=WrappedClientServiceListenerDirector(listener)
		listener2.__disown__()
	
	return async_call(self._AsyncConnectService,(url, username, credentials, listener2, "", adjust_timeout(timeout)), AsyncStubReturnDirectorImpl, handler)

def AsyncDisconnectService(self, stub, handler):
	from .RobotRaconteurPythonUtil import async_call, AsyncVoidNoErrReturnDirectorImpl
	return async_call(self._AsyncDisconnectService,(stub.rrinnerstub,), AsyncVoidNoErrReturnDirectorImpl,handler,noerror=True)

@classproperty.classpropertyreadonly
def s(self):
	return RobotRaconteurNode._get_s()

def RegisterServiceType(self, d):
	self._RegisterServiceType(d)

def RegisterServiceTypeFromFile(self, file_name):
	from .RobotRaconteurPythonUtil import ReadServiceDefinitionFile
	d = ReadServiceDefinitionFile(file_name)
	self._RegisterServiceType(str(d))
	
def GetServiceType(self, name):
	return self._GetServiceType(name)

def GetRegisteredServiceTypes(self):
	return self._GetRegisteredServiceTypes()
	
def GetPulledServiceTypes(self,obj):
	if (hasattr(obj,'rrinnerstub')):
		obj=obj.rrinnerstub
	return self._GetPulledServiceTypes(obj)
		
def GetPulledServiceType(self,obj,servicename):
	if (hasattr(obj,'rrinnerstub')):
		obj=obj.rrinnerstub
	return self._GetPulledServiceType(obj,servicename)

def GetServicePath(self,obj):
	if (hasattr(obj,'rrinnerstub')):
		obj=obj.rrinnerstub
	return self._GetServicePath(obj)
		
def NewStructure(self,type,obj=None):
	from .RobotRaconteurPythonUtil import NewStructure
	return NewStructure(type,obj,self)

def GetPodDType(self,type,obj=None):
	from .RobotRaconteurPythonUtil import GetPodDType
	return GetPodDType(type,obj,self)

def GetNamedArrayDType(self,type,obj=None):
	from .RobotRaconteurPythonUtil import GetNamedArrayDType
	return GetNamedArrayDType(type,obj,self)
	
def NamedArrayToArray(self,named_array):
	from .RobotRaconteurPythonUtil import NamedArrayToArray
	return NamedArrayToArray(named_array)

def ArrayToNamedArray(self,a,named_array_dt):
	from .RobotRaconteurPythonUtil import ArrayToNamedArray
	return ArrayToNamedArray(a,named_array_dt)

def AsyncRequestObjectLock(self,obj,flags,handler,timeout=RR_TIMEOUT_INFINITE):
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout, AsyncStringReturnDirectorImpl
	return async_call(self._AsyncRequestObjectLock,(obj.rrinnerstub,flags,adjust_timeout(timeout)),AsyncStringReturnDirectorImpl,handler)
				
def AsyncReleaseObjectLock(self,obj,handler,timeout=RR_TIMEOUT_INFINITE):
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout, AsyncStringReturnDirectorImpl
	return async_call(self._AsyncReleaseObjectLock,(obj.rrinnerstub,adjust_timeout(timeout)),AsyncStringReturnDirectorImpl,handler)

def GetServiceAttributes(self,obj):
	from .RobotRaconteurPythonUtil import UnpackMessageElement
	return UnpackMessageElement(self._GetServiceAttributes(obj.rrinnerstub),"varvalue{string} value",None,self)
	
NodeID = property(lambda self: self._NodeID(), lambda self,nodeid: self._SetNodeID(nodeid))
NodeName =property(lambda self: self._NodeName(), lambda self,nodename: self._SetNodeName(nodename))
	
RequestTimeout = property(lambda self : self._GetRequestTimeout()/1000.0, lambda self,t : self._SetRequestTimeout(t*1000))
TransportInactivityTimeout = property(lambda self : self._GetTransportInactivityTimeout()/1000.0, lambda self,t : self._SetTransportInactivityTimeout(t*1000))
EndpointInactivityTimeout = property(lambda self : self._GetEndpointInactivityTimeout()/1000.0, lambda self,t : self._SetEndpointInactivityTimeout(t*1000))
MemoryMaxTransferSize = property(lambda self: self._GetMemoryMaxTransferSize(), lambda self,m: self._SetMemoryMaxTransferSize(m))
NodeDiscoveryMaxCacheCount = property(lambda self: self._GetNodeDiscoveryMaxCacheCount(), lambda self,c: self._SetNodeDiscoveryMaxCacheCount(c))

def GetConstants(self,servicetype, obj=None):
	from .RobotRaconteurPythonUtil import ServiceDefinitionConstants
	if obj is None:
		d=self.GetServiceType(servicetype)
	else:
		d=self.GetPulledServiceType(obj,servicetype)
	return ServiceDefinitionConstants(d,self,obj)
		
def GetExceptionType(self, exceptionname, obj=None):
	from .RobotRaconteurPythonUtil import SplitQualifiedName
	from .RobotRaconteurPythonError import GetExceptionType
	t=SplitQualifiedName(exceptionname)
	if (obj is None):
		d=self.GetServiceType(t[0])
	else:
		d=self.GetPulledServiceType(obj,t[0])
	if (not t[1] in d.Exceptions): raise Exception('Invalid exception type')
	return GetExceptionType(exceptionname)
		
def AsyncFindObjectType(self,obj,member,handler,timeout=RR_TIMEOUT_INFINITE):
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout, AsyncStringReturnDirectorImpl
	return async_call(self._AsyncFindObjectType,(obj.rrinnerstub,member,adjust_timeout(timeout)),AsyncStringReturnDirectorImpl,handler)
			
def AsyncFindObjectTypeInd(self,obj,member,ind,handler,timeout=RR_TIMEOUT_INFINITE):
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout, AsyncStringReturnDirectorImpl
	return async_call(self._AsyncFindObjectType,(obj.rrinnerstub,member,ind,adjust_timeout(timeout)),AsyncStringReturnDirectorImpl,handler)

def SetExceptionHandler(self, handler):
	from .RobotRaconteurPythonUtil import ExceptionHandlerDirectorImpl
	if (handler is None):
		self._ClearExceptionHandler()
	else:
		d=ExceptionHandlerDirectorImpl(handler)
		d.__disown__()
		self._SetExceptionHandler(d,0)
			
def CreateTimer(self,period,handler,oneshot=False):
	from .RobotRaconteurPythonUtil import AsyncTimerEventReturnDirectorImpl
	
	handler2=AsyncTimerEventReturnDirectorImpl(handler)
	handler2.__disown__()
	ret= self._CreateTimer(period,oneshot,handler2,0)
		
	return ret
		
def PostToThreadPool(self, handler):
	from .RobotRaconteurPythonUtil import async_call, AsyncVoidNoErrReturnDirectorImpl
	return async_call(self._PostToThreadPool,(), AsyncVoidNoErrReturnDirectorImpl,handler,noerror=True)

def AsyncSleep(self, d, handler):
	from .RobotRaconteurPythonUtil import async_call, AsyncVoidNoErrReturnDirectorImpl
	return async_call(self._AsyncSleep,(d,), AsyncVoidNoErrReturnDirectorImpl,handler,noerror=True)

RobotRaconteurVersion = property(lambda self: self._GetRobotRaconteurVersion())

def NowUTC(self):
	return self._NowUTC()

def Shutdown(self):
	self._Shutdown()

def SubscribeService(self, service_types, filter_=None):
	from .RobotRaconteurPythonUtil import SubscribeService
	return SubscribeService(self, service_types, filter_)

def SubscribeServiceInfo2(self, service_types, filter_=None):
	from .RobotRaconteurPythonUtil import SubscribeServiceInfo2
	return SubscribeServiceInfo2(self, service_types, filter_)

%}


}