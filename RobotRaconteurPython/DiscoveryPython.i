%include "Discovery.i"

%extend RobotRaconteur::RobotRaconteurNode
{
%pythoncode %{
def AsyncFindServiceByType(self,servicetype,transportschemes,handler,timeout=5):
	"""
	Asynchronously use discovery to find availabe services by service type

	Same as FindServiceByType() but returns asynchronously

	If ``handler`` is None, returns an awaitable future.

	:param servicetype: The service type to find, ie \"com.robotraconteur.robotics.robot.Robot\"
	:type servicetype: str
	:param transportschemes: A list of transport types to search, ie \"rr+tcp\", \"rr+local\", \"rrs+tcp\", etc
	:type transportschemes: List[str]
	:param handler: Handler to call on completion
	:type handler: Callable[[List[ServiceInfo2]],None]
	:param timeout: Timeout in seconds. Using a timeout greater than 5 seconds is not recommended.
	:type timeout: float
	"""
	class ServiceInfo2Director(AsyncServiceInfo2VectorReturnDirector):
		def __init__(self,handler):			
			super(ServiceInfo2Director,self).__init__()
			self._handler=handler
		
		def handler(self,info1):
			from .RobotRaconteurPythonUtil import ServiceInfo2
			ret=[]			
			for e in info1:
				ret.append(ServiceInfo2(e))
			self._handler(ret)
	
	ts2=vectorstring()
	for t in transportschemes:
		ts2.push_back(t)
	
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout
	return async_call(AsyncWrappedFindServiceByType,(self,servicetype,ts2,adjust_timeout(timeout)),ServiceInfo2Director,handler,False)
	
%}
}

//Find Nodes

%extend RobotRaconteur::RobotRaconteurNode
{

%pythoncode %{
	
def AsyncFindNodeByID(self,id,transportschemes,handler,timeout=5):
	"""
	Asynchronously finds nodes on the network with the specified NodeID

	Same as FindNodeByID() but returns asynchronously

	If ``handler`` is None, returns an awaitable future.

	:param id: The NodeID to find
	:type id: RobotRaconteur.NodeID
	:param transportschemes: A list of transport types to search, ie \"rr+tcp\", \"rr+local\", \"rrs+tcp\", etc
	:type transportschemes: List[str]
	:param handler: Handler to call on completion
	:type handler: Callable[List[NodeInfo2],None]
	:param timeout: Timeout in seconds. Using a timeout greater than 5 seconds is not recommended.
	:type timeout: float
	"""
	class NodeInfo2Director(AsyncNodeInfo2VectorReturnDirector):
		def __init__(self,handler):
			super(NodeInfo2Director,self).__init__()
			self._handler=handler
		
		def handler(self,info1):
			from .RobotRaconteurPythonUtil import NodeInfo2
			ret=[]			
			for e in info1:
				ret.append(NodeInfo2(e))
			self._handler(ret)
	ts2=vectorstring()
	for t in transportschemes:
		ts2.push_back(t)
	
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout
	return async_call(AsyncWrappedFindNodeByID,(self,id,ts2,adjust_timeout(timeout)),NodeInfo2Director,handler,False)
	
def AsyncFindNodeByName(self,name,transportschemes,handler,timeout=5):
	"""
	Asynchronously finds nodes on the network with the specified NodeName

	Same as FindNodeByName() but returns asynchronously

	If ``handler`` is None, returns an awaitable future.

	:param name: The NodeName to find
	:type name: str
	:param transportschemes: A list of transport types to search, ie \"rr+tcp\", \"rr+local\", \"rrs+tcp\", etc
	:type transportschemes: List[str]
	:param handler: Handler to call on completion
	:type handler: Callable[List[NodeInfo2],None]
	:param timeout: Timeout in seconds. Using a timeout greater than 5 seconds is not recommended.
	:type timeout: float
	"""
	class NodeInfo2Director(AsyncNodeInfo2VectorReturnDirector):
		def __init__(self,handler):
			super(NodeInfo2Director,self).__init__()
			self._handler=handler
		
		def handler(self,info1):
			from .RobotRaconteurPythonUtil import NodeInfo2
			ret=[]			
			for e in info1:
				ret.append(NodeInfo2(e))
			self._handler(ret)
	ts2=vectorstring()
	for t in transportschemes:
		ts2.push_back(t)
	
	from .RobotRaconteurPythonUtil import async_call, adjust_timeout
	return async_call(AsyncWrappedFindNodeByName,(self,name,ts2,adjust_timeout(timeout)),NodeInfo2Director,handler,False)	

def AsyncUpdateDetectedNodes(self,schemes,handler,timeout=5):
	"""
	Asynchronously update the detected nodes cache

	Same as UpdateDetectedNodes() but returns asynchronously

	If ``handler`` is None, returns an awaitable future.

	:param schemes: A list of transport schemes, ie \"rr+tcp\", \"rr+local\", etc. to update.
	:type schemes: List[str]
	:param handler: The handler to call on completion
	:type handler: Callable[[],None]
	:param timeout: The timeout for the operation in seconds. This function will often run
	 for the full timeout, so values less than 5 seconds are recommended.
	:type timeout: float
	"""
	ts2=vectorstring()
	for t in schemes:
		ts2.push_back(t)

	from .RobotRaconteurPythonUtil import async_call, adjust_timeout, AsyncVoidNoErrReturnDirectorImpl
	return async_call(AsyncWrappedUpdateDetectedNodes,(self,ts2, adjust_timeout(timeout),), AsyncVoidNoErrReturnDirectorImpl,handler,noerror=True)

def GetDetectedNodes(self):
	"""
	Get the nodes currently detected by Transports

	Transports configured to listen for node discovery send detected node
	information to the parent node, where it is stored. Normally this information
	will expire after one minute, and needs to be constantly refreshed.

	This node information is not verified. It is the raw discovery 
	information received by the transports. Verification is done
	when the node is interrogated for service information.

	:return: List of detected NodeID
	:rtype: List[RobotRaconteur.NodeID]
	"""
	o1=WrappedGetDetectedNodes(self)
	return [NodeID(x) for x in o1]	

%}
}