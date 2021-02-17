# Copyright 2011-2020 Wason Technology, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from __future__ import absolute_import

from . import RobotRaconteurPython
from .RobotRaconteurPythonDataTypes import *
from . import RobotRaconteurPythonError
import operator
import traceback
import threading
import functools
import sys
import warnings
import weakref;
import codecs
import numbers
import os
from functools import partial
from RobotRaconteur.RobotRaconteurPython import DataTypes_ContainerTypes_generator
import numpy
import itertools

if (sys.version_info  > (3,0)):
    from builtins import property
    from functools import reduce
else:
    from __builtin__ import property

import numpy

def SplitQualifiedName(name):
    pos=name.rfind('.')
    if (pos==-1): raise Exception("Name is not qualified")
    return (name[0:pos],name[pos+1:])

def FindStructureByName(l,name):
    for i in l:
        if (i.Name==name):
            return i
    raise RobotRaconteurPythonError.ServiceException("Structure " + name + " not found")

def FindMemberByName(l,name):
    for i in l:
        if (i.Name==name):
            return i
    raise RobotRaconteurPythonError.MemberNotFoundException("Member " + name + " not found")

def FindMessageElementByName(l,name):
    return RobotRaconteurPython.MessageElement.FindElement(l,name)

def PackMessageElement(data,type1,obj=None,node=None):

    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub

    if (isinstance(type1,str)):
        if (len(type1.split())==1): type1=type1+ " value"
        type2=RobotRaconteurPython.TypeDefinition()
        type2.FromString(type1)
        type1=type2    

    return RobotRaconteurPython._PackMessageElement(data, type1, obj, node)

def UnpackMessageElement(element,type1=None,obj=None,node=None):
    
    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub
    
    if (not type1 is None):
        if (isinstance(type1,str)):
            if (len(type1.split())==1): type1=type1+ " value"
            type2=RobotRaconteurPython.TypeDefinition()
            type2.FromString(type1)
            type1=type2
            
    return RobotRaconteurPython._UnpackMessageElement(element, type1, obj, node)


def PackToRRArray(array,type1,destrrarray=None):

    return RobotRaconteurPython._PackToRRArray(array,type1,destrrarray)

def UnpackFromRRArray(rrarray,type1=None):

    if (rrarray is None): return None
    
    return RobotRaconteurPython._UnpackFromRRArray(rrarray, type1)

def CreateStructureType(name, dict_):
    def struct_init(s):
        for k,v in dict_.items():
            init_type, init_args = v
            if init_type is None:
                setattr(s,k,None)
            else:
                setattr(s,k,init_type(*init_args))
    slots = list(dict_.keys())
    return type(name, (RobotRaconteurStructure,), {'__init__': struct_init, "__slots__": slots})

def CreateZeroArray(dtype, dims):
    if dims is None:
        return numpy.zeros((0,),dtype)
    else:
        return numpy.zeros(dims, dtype)

def NewStructure(StructType,obj=None,node=None):
    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub
    return RobotRaconteurPython._NewStructure(StructType, obj, node)

def GetStructureType(StructType,obj=None,node=None):
    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub
    return RobotRaconteurPython._GetStructureType(StructType, obj, node)

def GetPodDType(pod_type,obj=None,node=None):
    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub
    return RobotRaconteurPython._GetNumPyDescrForType(pod_type, obj, node)

def GetNamedArrayDType(namedarray_type,obj=None,node=None):
    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub
    return RobotRaconteurPython._GetNumPyDescrForType(namedarray_type, obj, node)

def GetPrimitiveDTypeFromNamedArrayDType(dt):
    prim_dt = None
    count = 0
    for dt_i in dt.fields.values():              
        if len(dt_i[0].shape) == 0:
            prim_dt_i = dt_i[0].type
            dt_i_2 = dt_i[0]
            count_i = 1
        else:            
            prim_dt_i = dt_i[0].subdtype[0].type
            dt_i_2 = dt_i[0].subdtype[0]
            count_i = dt_i[0].shape[0]
                        
        if (prim_dt_i == numpy.void):            
            prim_dt_i, n = GetPrimitiveDTypeFromNamedArrayDType(dt_i_2)
            count_i *= n
        
        if prim_dt is None:
            prim_dt = prim_dt_i
        else:            
            assert prim_dt == prim_dt_i
        
        count += count_i
        
    return prim_dt, count

def NamedArrayToArray(named_array):    
    prim_dt, _ = GetPrimitiveDTypeFromNamedArrayDType(named_array.dtype)
    a = numpy.ascontiguousarray(named_array.reshape(named_array.shape + (1,)))    
    return a.view(prim_dt) 

def ArrayToNamedArray(a, named_array_dt):
    prim_dt, elem_count = GetPrimitiveDTypeFromNamedArrayDType(named_array_dt)
    a = numpy.ascontiguousarray(a)
    assert a.dtype == prim_dt, "Array type must match named array element type"
    assert a.shape[-1] == elem_count, "Last dimension must match named array size"
    b = a.view(named_array_dt)
    if len(b.shape) > 1 and b.shape[-1] == 1:
        return b.reshape(a.shape[0:-1],order="C")
    return b

def InitStub(stub):
    odef=stub.RR_objecttype
    mdict={}

    for i in range(len(odef.Members)):
        m=odef.Members[i]
        if (isinstance(m,RobotRaconteurPython.PropertyDefinition)):
            
            def inner_async_prop(m1):
                fget=lambda  self,handler,timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE : stub_async_getproperty(stub,m1.Name,m1,handler,timeout)
                fset=lambda self, value,handler,timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE : stub_async_setproperty(stub,m1.Name,m1,value,handler,timeout)
                return fget, fset
            p1_async=inner_async_prop(m)

            mdict['async_get_' + m.Name]=p1_async[0]
            mdict['async_set_' + m.Name]=p1_async[1]


        if (isinstance(m,RobotRaconteurPython.FunctionDefinition)):
            
            def inner_async_func(m1):              
                if not m1.IsGenerator():
                    if (m1.ReturnType.Type==RobotRaconteurPython.DataTypes_void_t):
                        f=lambda self,*args : stub_async_functioncallvoid(stub,m1.Name,m1,*args)
                    else:
                        f=lambda self,*args : stub_async_functioncall(stub,m1.Name,m1,*args)
                    return f
                else:
                    return lambda self,*args : stub_async_functioncallgenerator(stub,m1.Name,m1,*args)

            f1_async=inner_async_func(m)
            mdict['async_' + m.Name]=f1_async

        if (isinstance(m,RobotRaconteurPython.EventDefinition)):
            def new_evt_hook():
                evt=EventHook()          
                fget=lambda self : evt
                def fset(self,value):
                    if (value is not evt):
                        raise RuntimeError("Invalid operation")
                return property(fget, fset)            
            mdict[m.Name]=new_evt_hook()

        if (isinstance(m,RobotRaconteurPython.ObjRefDefinition)):
            
            def inner_async_objref(m1):
                if(m1.ArrayType != RobotRaconteurPython.DataTypes_ArrayTypes_none or m1.ContainerType != RobotRaconteurPython.DataTypes_ContainerTypes_none):
                    f=lambda self,index,handler,timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE : stub_async_objref(stub,m1.Name,index,handler,timeout)
                else:
                    f=lambda self,handler,timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE : stub_async_objref(stub,m1.Name,None,handler,timeout)
                return f
            f1=inner_async_objref(m)
            mdict['async_get_%s' % m.Name]=f1

        if (isinstance(m,RobotRaconteurPython.PipeDefinition)):
            def inner_pipe(m1):
                innerp=stub.GetPipe(m1.Name)
                outerp=Pipe(innerp)
                fget=lambda self : outerp
                return property(fget)
            p2=inner_pipe(m)
            mdict[m.Name]=p2

        if (isinstance(m,RobotRaconteurPython.CallbackDefinition)):
            def new_cb_client():
                cb=CallbackClient()
                fget = lambda self: cb
                return property(fget)            
            mdict[m.Name]=new_cb_client()

        if (isinstance(m,RobotRaconteurPython.WireDefinition)):
            def inner_wire(m1):
                innerw=stub.GetWire(m1.Name)
                outerw=Wire(innerw)
                fget=lambda self : outerw
                return property(fget)
            w=inner_wire(m)
            mdict[m.Name]=w

        
    mdict["__slots__"] = ["rrinnerstub","rrlock"]
    outerstub_type=type(str(odef.Name),(ServiceStub,),mdict)
    outerstub=outerstub_type()

    for i in range(len(odef.Members)):
        m=odef.Members[i]

        if (isinstance(m,RobotRaconteurPython.PipeDefinition)):
            p=getattr(outerstub,m.Name)
            p._obj=outerstub

        if (isinstance(m,RobotRaconteurPython.WireDefinition)):
            w=getattr(outerstub,m.Name)
            w._obj=outerstub

    director=WrappedServiceStubDirectorPython(outerstub,stub)
    stub.SetRRDirector(director,0)
    director.__disown__()

    stub.SetPyStub(outerstub)
   
    outerstub.rrinnerstub=stub
    outerstub.rrlock=threading.RLock()
    return outerstub

def check_member_args(name, param_types, args, isasync=False):
    expected_args_len = len(param_types)
    if isasync:
        expected_args_len += 1
    if len(param_types) > 0 and param_types[-1].ContainerType == RobotRaconteurPython.DataTypes_ContainerTypes_generator:
        expected_args_len -= 1        
    if expected_args_len != len(args):
            raise TypeError("%s() expects exactly %d arguments (%d given)" % (name, expected_args_len, len(args)))

class AsyncRequestDirectorImpl(RobotRaconteurPython.AsyncRequestDirector):
    def __init__(self,handler,isvoid,Type,stub,node):
        super(AsyncRequestDirectorImpl,self).__init__()
        self._handler=handler
        self._isvoid=isvoid
        self._Type=Type
        self._node=node
        self._stub=stub

    def handler(self,m,error_info):

        if (self._isvoid):
            if (error_info.error_code!=0):
                err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
                self._handler(err)
                return
            else:
                self._handler(None)
        else:
            if (error_info.error_code!=0):
                err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
                self._handler(None,err)
                return
            else: 
                try:
                    a=UnpackMessageElement(m,self._Type,self._stub,self._node)
                except Exception as err:
                    self._handler(None,err)
                    return
                self._handler(a,None)
                return


def stub_async_getproperty(stub,name,type1,handler,timeout=-1):
    return async_call(stub.async_PropertyGet,(name,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(False,type1.Type,stub,stub.RRGetNode()))

def stub_async_setproperty(stub,name,type1,value,handler,timeout=-1):
    pvalue=PackMessageElement(value,type1.Type,stub)
    return async_call(stub.async_PropertySet,(name,pvalue,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(True,type1.Type,stub,stub.RRGetNode()))


def stub_async_functioncall(stub,name,type1,*args):
    check_member_args(name, type1.Parameters, args, True)
    m=RobotRaconteurPython.vectorptr_messageelement()
    i=0
    for p in type1.Parameters:
        a=PackMessageElement(args[i],p,stub)
        a.ElementName = p.Name
        m.append(a)
        i+=1
    handler=args[i]
    if (len(args) > i+1):
        timeout=args[i+1]
    else:
        timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE
    return async_call(stub.async_FunctionCall,(name,m,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(False,type1.ReturnType,stub,stub.RRGetNode()))

def stub_async_functioncallvoid(stub,name,type1,*args):
    check_member_args(name, type1.Parameters, args, True)
    m=RobotRaconteurPython.vectorptr_messageelement()
    i=0
    for p in type1.Parameters:
        a=PackMessageElement(args[i],p,stub)
        a.ElementName = p.Name
        m.append(a)
        i+=1
    handler=args[i]
    if (len(args) > i+1):
        timeout=args[i+1]
    else:
        timeout=-1

    return async_call(stub.async_FunctionCall,(name,m,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(True,type1.ReturnType,stub,stub.RRGetNode()))

def stub_async_functioncallgenerator(stub,name,type1,*args):
    check_member_args(name, type1.Parameters, args, True)
    m=RobotRaconteurPython.vectorptr_messageelement()
    i=0
    param_type = None
    for p in type1.Parameters:
        if (p.ContainerType != RobotRaconteurPython.DataTypes_ContainerTypes_generator):
            a=PackMessageElement(args[i],p,stub)
            a.ElementName = p.Name
            m.append(a)
            i+=1
        else:
            param_type = p
    handler=args[i]
    if (len(args) > i+1):
        timeout=args[i+1]
    else:
        timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE
    return async_call(stub.async_GeneratorFunctionCall,(name,m,adjust_timeout(timeout)),AsyncGeneratorClientReturnDirectorImpl,handler,directorargs=(type1.ReturnType,param_type,stub,stub.RRGetNode()))


def stub_async_objref(stub,name,index,handler,timeout=-1):
    if (index is None):
        return async_call(stub.async_FindObjRef,(name,adjust_timeout(timeout)),AsyncStubReturnDirectorImpl,handler)
    else:
        return async_call(stub.async_FindObjRef,(name,str(index),adjust_timeout(timeout)),AsyncStubReturnDirectorImpl,handler)

class WrappedServiceStubDirectorPython(RobotRaconteurPython.WrappedServiceStubDirector):
    def __init__(self,stub,innerstub):
        self.stub=stub;
        self.innerstub=innerstub
        super(WrappedServiceStubDirectorPython, self).__init__()

    def DispatchEvent(self, name, args1):
        try:
            type1=FindMemberByName(self.innerstub.RR_objecttype.Members,name)
            #type1=[e for e in self.innerstub.RR_objecttype.Members if e.Name == name][0]
            args=[]        
            type2=RobotRaconteurPython.MemberDefinitionUtil.ToEvent(type1)
            for p in type2.Parameters:
                m=FindMessageElementByName(args1,p.Name)            
                a=UnpackMessageElement(m,p,self.stub,node=self.innerstub.RRGetNode())
                args.append(a)
            getattr(self.stub,name).fire(*args)
        except:
            traceback.print_exc()



    def CallbackCall(self, name, args1):
        try:
            type1=FindMemberByName(self.innerstub.RR_objecttype.Members,name)
            #type1=[e for e in self.innerstub.RR_objecttype.Members if e.Name == name][0]
            args=[]        
    
            type2=RobotRaconteurPython.MemberDefinitionUtil.ToCallback(type1)
            for p in type2.Parameters:
                m=FindMessageElementByName(args1,p.Name)
                
                a=UnpackMessageElement(m,p,self.stub,self.innerstub.RRGetNode())
                args.append(a)
            ret=getattr(self.stub,name).Function(*args)
    
            if (ret is None):
                m=RobotRaconteurPython.MessageElement()
                m.ElementName="return"
                m.ElementType=RobotRaconteurPython.DataTypes_void_t
                return m
            return PackMessageElement(ret,type2.ReturnType,self.stub,self.innerstub.RRGetNode())
        except:
            traceback.print_exc()

class ServiceStub(object):
    __slots__ = ["rrinnerstub","rrlock","__weakref__"]
    pass

class CallbackClient(object):
    __slots__ = ["Function","__weakref__"]
    def __init__(self):
        self.Function=None

class PipeEndpoint(object):
    """
    PipeEndpoint()

    Pipe endpoint used to transmit reliable or unreliable data streams

    Pipe endpoints are used to communicate data between connected pipe members.
    See Pipe for more information on pipe members.

    Pipe endpoints are created by clients using the Pipe.Connect() or Pipe.AsyncConnect()
    functions. Services receive incoming pipe endpoint connection requests through a 
    callback function specified using the Pipe.PipeConnectCallback property. Services
    may also use the PipeBroadcaster class to automate managing pipe endpoint lifecycles and
    sending packets to all connected client endpoints.

    Pipe endpoints are *indexed*, meaning that more than one pipe endpoint pair can be created
    using the same member. This means that multiple data streams can be created independent of
    each other between the client and service using the same member.

    Pipes send reliable packet streams between connected client/service endpoint pairs.
    Packets are sent using the SendPacket() or AsyncSendPacket() functions. Packets
    are read from the receive queue using the ReceivePacket(), ReceivePacketWait(), 
    TryReceivePacketWait(), TryReceivePacketWait(), or PeekNextPacket(). The endpoint is closed
    using the Close() or AsyncClose() function.

    This class is instantiated by the Pipe class. It should not be instantiated
    by the user.
    """
    __slots__ = ["__innerpipe", "__type", "PipeEndpointClosedCallback", "_PacketReceivedEvent", "_PacketAckReceivedEvent", "__obj","__weakref__"]
    def __init__(self,innerpipe, type, obj=None):
        self.__innerpipe=innerpipe
        self.__type=type
        self.PipeEndpointClosedCallback=None
        """
        (Callable[[RobotRaconteur.PipeEndpoint],None]) The function to invoke when the pipe endpoint has been closed.
        """
        self._PacketReceivedEvent=EventHook()
        self._PacketAckReceivedEvent=EventHook()
        self.__obj=obj

    @property
    def Index(self):
        """
        The pipe endpoint index used when endpoint connected

        :rtype: int
        """
        return self.__innerpipe.GetIndex()

    @property
    def Endpoint(self):
        """
        the endpoint associated with the ClientContext or ServerEndpoint
        associated with the pipe endpoint.

        :rtype: int
        """
        return self.__innerpipe.GetEndpoint()

    @property
    def Available(self):
        """
        Return number of packets in the receive queue

        Invalid for *writeonly* pipes.

        :rtype: int
        """
        return self.__innerpipe.Available()

    @property
    def IsUnreliable(self):
        """
        Get if pipe endpoint is unreliable

        Pipe members may be declared as *unreliable* using member modifiers in the
        service definition. Pipes confirm unreliable operation when pipe endpoints are connected.

        :rtype: bool
        """
        return self.__innerpipe.IsUnreliable()

    @property
    def Direction(self):
        """
        The direction of the pipe

        Pipes may be declared "readonly" or "writeonly" in the service definition file. (If neither
        is specified, the pipe is assumed to be full duplex.) "readonly" pipes may only send packets from
        service to client. "writeonly" pipes may only send packets from client to service.

        See ``MemberDefinition_Direction`` constants for possible return values.

        :rtype: int
        """
        return self.__innerpipe.Direction()

    @property
    def RequestPacketAck(self):
        """
        Get if pipe endpoint should request packet acks

        Packet acks are generated by receiving endpoints to inform the sender that
        a packet has been received. The ack contains the packet index, the sequence number
        of the packet. Packet acks are used for flow control by PipeBroadcaster.

        :rtype: bool
        """
        return self.__innerpipe.GetRequestPacketAck()

    @RequestPacketAck.setter
    def RequestPacketAck(self,value):
        self.__innerpipe.SetRequestPacketAck(value)

    @property
    def IgnoreReceived(self):
        """
        Set if pipe endpoint is ignoring incoming packets

        If true, pipe endpoint is ignoring incoming packets and is not adding
        incoming packets to the receive queue.

        :rtype: bool
        """
        return self.__innerpipe.GetIgnoreReceived()

    @IgnoreReceived.setter
    def IgnoreReceived(self,value):
        self.__innerpipe.SetIgnoreReceived(value)

    def AsyncClose(self,handler,timeout=2):
        """
        Asynchronously close the pipe endpoint

        Same as Close() but returns asynchronously

        If ``handler`` is None, returns an awaitable future.

        :param handler: A handler function to call on completion, possibly with an exception
        :type handler: Callable[[Exception],None]
        :param timeout: Timeout in seconds, or -1 for no timeout
        :type timeout: float
        """
        return async_call(self.__innerpipe.AsyncClose,(adjust_timeout(timeout),),AsyncVoidReturnDirectorImpl,handler)

    def AsyncSendPacket(self, packet, handler):
        """
        Send a packet to the peer endpoint asynchronously

        Same as SendPacket(), but returns asynchronously.

        If ``handler`` is None, returns an awaitable future.

        :param packet: The packet to send
        :param handler: A handler function to receive the sent packet number or an exception
        :type handler: Callable[[Exception],None]
        """
        m=PackMessageElement(packet,self.__type,self.__obj,self.__innerpipe.GetNode())
        return async_call(self.__innerpipe.AsyncSendPacket,(m,),AsyncUInt32ReturnDirectorImpl,handler)

    def ReceivePacket(self):
        """
        Receive the next packet in the receive queue

        Receive the next packet from the receive queue. This function will throw an
        InvalidOperationException if there are no packets in the receive queue. Use
        ReceivePacketWait() to block until a packet has been received.

        :return: The received packet
        """
        m=self.__innerpipe.ReceivePacket()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerpipe.GetNode())

    def PeekNextPacket(self):
        """
        Peeks the next packet in the receive queue

        Returns the first packet in the receive queue, but does not remove it from
        the queue. Throws an InvalidOperationException if there are no packets in the
        receive queue.

        :return: The next packet in the receive queue
        """
        m=self.__innerpipe.PeekNextPacket()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerpipe.GetNode())

    def TryReceivePacketWait(self, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE, peek=False):
        #TODO: Add timeout back
        m=RobotRaconteurPython.MessageElement()
        r=self.__innerpipe.TryReceivePacket(m, peek)
        return (r, UnpackMessageElement(m,self.__type,self.__obj,self.__innerpipe.GetNode()))
    
    @property
    def PacketReceivedEvent(self):
        """
        Event hook for received packets. Use to add handlers to be called
        when packets are received by the endpoint.

        .. code-block:: python

           def my_handler(ep):
              # Receive packets
              while ep.Available > 0:
                  packet = ep.ReceivePacket()
                  # Do something with packet

           my_endpoint.PacketReceivedEvent += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.PipeEndpoint],None]``

        """

        return self._PacketReceivedEvent
    
    @PacketReceivedEvent.setter
    def PacketReceivedEvent(self, evt):
        if (evt is not self._PacketReceivedEvent):
            raise RuntimeError("Invalid operation")
    
    @property
    def PacketAckReceivedEvent(self):
        """
        Event hook for received packets. Use to add handlers to be called
        when packets are received by the endpoint.

        .. code-block:: python

           def my_ack_handler(ep, packet_num):
              # Do something with packet_num info
              pass

           my_endpoint.PacketAckReceivedEvent += my_ack_handler

        Handler must have signature ``Callable[[RobotRaconteur.PipeEndpoint,T],None]``

        """
        return self._PacketAckReceivedEvent
    
    @PacketAckReceivedEvent.setter 
    def PacketAckReceivedEvent(self, evt):
        if (evt is not self._PacketAckReceivedEvent):
            raise RuntimeError("Invalid operation")

class PipeEndpointDirector(RobotRaconteurPython.WrappedPipeEndpointDirector):
    def __init__(self,endpoint):
        self.__endpoint=endpoint
        super(PipeEndpointDirector, self).__init__()

    def PipeEndpointClosedCallback(self):

        if (not self.__endpoint.PipeEndpointClosedCallback is None):
            self.__endpoint.PipeEndpointClosedCallback(self.__endpoint)


    def PacketReceivedEvent(self):

        self.__endpoint.PacketReceivedEvent.fire(self.__endpoint)


    def PacketAckReceivedEvent(self,packetnum):

        self.__endpoint.PacketAckReceivedEvent.fire(self.__endpoint,packetnum)


class PipeAsyncConnectHandlerImpl(RobotRaconteurPython.AsyncPipeEndpointReturnDirector):
    def __init__(self,handler,innerpipe,obj):
        super(PipeAsyncConnectHandlerImpl,self).__init__()
        self._handler=handler
        self.__innerpipe=innerpipe
        self.__obj=obj

    def handler(self, innerendpoint, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(None,err)
            return
        try:            
            outerendpoint=PipeEndpoint(innerendpoint,self.__innerpipe.Type,self.__obj)
            director=PipeEndpointDirector(outerendpoint)
            innerendpoint.SetRRDirector(director,0)
            director.__disown__()
        except Exception as err2:
            self._handler(None, err2)
            return
        self._handler(outerendpoint, None)

class Pipe(object):
    """
    Pipe()

    "pipe" member type interface

    The Pipe class implements the "pipe" member type. Pipes are declared in service definition files
    using the "pipe" keyword within object declarations. Pipes provide reliable packet streaming between
    clients and services. They work by creating pipe endpoint pairs (peers), with one endpoint in the client,
    and one in the service. Packets are transmitted between endpoint pairs. Packets sent by one endpoint are received
    by the other, where they are placed in a receive queue. Received packets can then be retrieved from the receive queue.

    Pipe endpoints are created by the client using the Connect() or AsyncConnect() functions. Services receive
    incoming connection requests through a callback function. This callback is configured using the PipeConnectCallback
    property. Services may also use the PipeBroadcaster class to automate managing pipe endpoint lifecycles and
    sending packets to all connected client endpoints. If the PipeConnectCallback function is used, the service
    is responsible for keeping track of endpoints as the connect and disconnect. See PipeEndpoint for details
    on sending and receiving packets.

    Pipe endpoints are *indexed*, meaning that more than one endpoint pair can be created between the client and the service.

    Pipes may be *unreliable*, meaning that packets may arrive out of order or be dropped. Use IsUnreliable to check for
    unreliable pipes. The member modifier `unreliable` is used to specify that a pipe should be unreliable.

    Pipes may be declared *readonly* or *writeonly*. If neither is specified, the pipe is assumed to be full duplex. *readonly* 
    pipes may only send packets from service to client. *writeonly* pipes may only send packets from client to service. Use
    Direction to determine the direction of the pipe.

    The PipeBroadcaster is often used to simplify the use of Pipes. See PipeBroadcaster for more information.

    This class is instantiated by the node. It should not be instantiated by the user.
    """
    __slots__ = ["_innerpipe", "_obj","__weakref__"]
    def __init__(self,innerpipe,obj=None):
        self._innerpipe=innerpipe        
        self._obj=obj

    def AsyncConnect(self,*args):
        """
        AsyncConnect(index,handler,timeout=-1)

        Asynchronously connect a pipe endpoint.
        
        Same as Connect(), but returns asynchronously.

        Only valid on clients. Will throw InvalidOperationException on the service side.

        If ``handler`` is None, returns an awaitable future.

        :param index: The index of the pipe endpoint, or -1 to automatically select an index
        :type index: int
        :param handler: A handler function to receive the connected endpoint, or an exception
        :type handler: Callable[[PipeEndpoint,Exception],None]
        :param timeout: Timeout in seconds, or -1 for no timeout
        """
        if (isinstance(args[0], numbers.Number)):
            index=args[0]
            handler=args[1]
            if (len(args)>=3):
                timeout=args[2]
            else:
                timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE
        else:
            index=-1
            handler=args[0]
            if (len(args)>=2):
                timeout=args[1]
            else:
                timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE

        return async_call(self._innerpipe.AsyncConnect,(index,adjust_timeout(timeout)),PipeAsyncConnectHandlerImpl,handler,directorargs=(self._innerpipe,self._obj))

    @property
    def MemberName(self):
        """
        Get the member name of the pipe

        :rtype: str
        """
        return self._innerpipe.GetMemberName()        
    
    @property
    def Direction(self):
        """
        The direction of the pipe

        Pipes may be declared "readonly" or "writeonly" in the service definition file. (If neither
        is specified, the pipe is assumed to be full duplex.) "readonly" pipes may only send packets from
        service to client. "writeonly" pipes may only send packets from client to service.

        See ``MemberDefinition_Direction`` constants for possible return values.

        :rtype: int
        """
        return self._innerpipe.Direction()
    

class WireConnection(object):
    """
    WireConnection()

    Wire connection used to transmit "most recent" values

    Wire connections are used to transmit "most recent" values between connected
    wire members. See Wire for more information on wire members.

    Wire connections are created by clients using the Wire.Connect() or Wire.AsyncConnect()
    functions. Services receive incoming wire connection requests through a 
    callback function specified using the Wire.WireConnectCallback property. Services
    may also use the WireBroadcaster class to automate managing wire connection lifecycles and
    sending values to all connected clients, or use WireUnicastReceiver to receive an incoming
    value from the most recently connected client.

    Wire connections are used to transmit "most recent" values between clients and services. Connection
    the wire creates a connection pair, one in the client, and one in the service. Each wire connection 
    object has an InValue and an OutValue. Setting the OutValue of one will cause the specified value to
    be transmitted to the InValue of the peer. See Wire for more information.

    Values can optionally be specified to have a finite lifespan using InValueLifespan and
    OutValueLifespan. Lifespans can be used to prevent using old values that have
    not been recently updated.

    This class is instantiated by the Wire class. It should not be instantiated
    by the user.
    """
    __slots__ = ["__innerwire", "__type", "WireConnectionClosedCallback", "_WireValueChanged", "__obj","__weakref__"]
    def __init__(self,innerwire, type, obj=None):
        self.__innerwire=innerwire
        self.__type=type
        self.WireConnectionClosedCallback=None
        self._WireValueChanged=EventHook()
        self.__obj=obj

    @property
    def Endpoint(self):
        """
        Get the Robot Raconteur node Endpoint ID

        Gets the endpoint associated with the ClientContext or ServerEndpoint
        associated with the wire connection.

        :rtype: int
        """
        return self.__innerwire.GetEndpoint()

    @property
    def Direction(self):
        """
        The direction of the wire

        Wires may be declared "readonly" or "writeonly" in the service definition file. (If neither
        is specified, the wire is assumed to be full duplex.) "readonly" wires may only send packets from
        service to client. "writeonly" wires may only send packets from client to service.

        See ``MemberDefinition_Direction`` constants for possible return values.

        :rtype: int
        """
        return self.__innerwire.Direction()

    def AsyncClose(self,handler,timeout=2):
        """
        Asynchronously close the wire connection

        Same as Close() but returns asynchronously

        :param handler: A handler function to call on completion, possibly with an exception
        :type handler: Callable[[Exception],None]
        :param timeout: Timeout in seconds, or -1 for infinite
        :type timeout: float
        """
        return async_call(self.__innerwire.AsyncClose,(adjust_timeout(timeout),),AsyncVoidReturnDirectorImpl,handler)

    @property
    def InValue(self):
        """
        Get the current InValue

        Gets the current InValue that was transmitted from the peer. Throws
        ValueNotSetException if no value has been received, or the most
        recent value lifespan has expired.
        """
        m=self.__innerwire.GetInValue()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerwire.GetNode())

    @property
    def OutValue(self):
        """
        Set the OutValue and transmit to the peer connection

        Sets the OutValue for the wire connection. The specified value will be
        transmitted to the peer, and will become the peers InValue. The transmission
        is unreliable, meaning that values may be dropped if newer values arrive.

        The most recent OutValue may also be read through this property.
        """
        m=self.__innerwire.GetOutValue()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerwire.GetNode())

    @OutValue.setter
    def OutValue(self,value):
        m=PackMessageElement(value,self.__type,self.__obj,self.__innerwire.GetNode())
        return self.__innerwire.SetOutValue(m)

    @property
    def LastValueReceivedTime(self):
        """
        Get the timestamp of the last received value

        Returns the timestamp of the value in the *senders* clock

        :rtype: RobotRaconteur.TimeSpec
        """
        return self.__innerwire.GetLastValueReceivedTime()

    @property
    def LastValueSentTime(self):
        """
        Get the timestamp of the last sent value

        Returns the timestamp of the last sent value in the *local* clock

        :rtype: RobotRaconteur.TimeSpec
        """
        return self.__innerwire.GetLastValueSentTime()

    @property
    def InValueValid(self):
        """
        Get if the InValue is valid

        The InValue is valid if a value has been received and 
        the value has not expired

        :rtype: bool
        """
        return self.__innerwire.GetInValueValid()

    @property
    def OutValueValid(self):
        """
        Get if the OutValue is valid

        The OutValue is valid if a value has been
        set using the OutValue property

        :rtype: bool
        """
        return self.__innerwire.GetOutValueValid()
        
    @property
    def IgnoreInValue(self):
        """
        Set whether wire connection should ignore incoming values

        Wire connections may optionally desire to ignore incoming values. This is useful if the connection
        is only being used to send out values, and received values may create a potential memory . If ignore is true, 
        incoming values will be discarded.

        :rtype: bool
        """
        return self.__innerwire.GetIgnoreInValue()

    @IgnoreInValue.setter
    def IgnoreInValue(self,value):
        self.__innerwire.SetIgnoreInValue(value)

    def TryGetInValue(self):
        """
        Try getting the InValue, returning true on success or false on failure

        Get the current InValue and InValue timestamp. Return true or false on
        success or failure instead of throwing exception.

        :return: Tuple of success, in value, and timespec
        :rtype: Tuple[bool,T,RobotRaconteur.TimeSpec]
        """
        res=self.__innerwire.TryGetInValue()
        if not res.res:
            return (False,None, None)
        return (True, UnpackMessageElement(res.value,self.__type,self.__obj,self.__innerwire.GetNode()), res.ts)

    def TryGetOutValue(self):
        """
        Try getting the OutValue, returning true on success or false on failure

        Get the current OutValue and OutValue timestamp. Return true or false on
        success and failure instead of throwing exception.

        :return: Tuple of success, out value, and timespec
        :rtype: Tuple[bool,T,RobotRaconteur.TimeSpec]
        """
        res=self.__innerwire.TryGetOutValue()
        if not res.res:
            return (False,None, None)
        return (True, UnpackMessageElement(res.value,self.__type,self.__obj,self.__innerwire.GetNode()), res.ts)


    @property
    def WireValueChanged(self):
        """
        Event hook for wire value change. Use to add handlers to be called
        when the InValue changes.

        .. code-block:: python

           def my_handler(con, value, ts):
              # Handle new value
              pass              

           my_wire_connection.WireValueChanged += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.WireConnection,T,RobotRaconteur.TimeSpec],None]``

        :rtype: RobotRaconteur.EventHook

        """
        return self._WireValueChanged
    
    @WireValueChanged.setter
    def WireValueChanged(self,evt):
        if (evt is not self._WireValueChanged):
            raise RuntimeError("Invalid operation")

    @property
    def InValueLifespan(self):
        """
        Set the lifespan of InValue

        InValue may optionally have a finite lifespan specified in seconds. Once
        the lifespan after reception has expired, the InValue is cleared and becomes invalid.
        Attempts to access InValue will result in ValueNotSetException.

        InValue lifespans may be used to avoid using a stale value received by the wire. If
        the lifespan is not set, the wire will continue to return the last received value, even
        if the value is old.

        Specify -1 for infinite lifespan.

        :rtype: float
        """
        t = self.__innerwire.GetInValueLifespan()
        if t < 0:
            return t
        return float(t) / 1000.0

    @InValueLifespan.setter
    def InValueLifespan(self, secs):
        if secs < 0:
            self.__innerwire.SetInValueLifespan(-1)
        else:
            self.__innerwire.SetInValueLifespan(int(secs*1000.0))

    @property
    def OutValueLifespan(self):
        """
        Set the lifespan of OutValue
        
        OutValue may optionally have a finite lifespan specified in seconds. Once
        the lifespan after sending has expired, the OutValue is cleared and becomes invalid.
        Attempts to access OutValue will result in ValueNotSetException.

        OutValue lifespans may be used to avoid using a stale value sent by the wire. If
        the lifespan is not set, the wire will continue to return the last sent value, even
        if the value is old.

        Specify -1 for infinite lifespan.

        :rtype: float
        """
        t = self.__innerwire.GetOutValueLifespan()
        if t < 0:
            return t
        return float(t) / 1000.0

    @OutValueLifespan.setter
    def OutValueLifespan(self, secs):
        if secs < 0:
            self.__innerwire.SetOutValueLifespan(-1)
        else:
            self.__innerwire.SetOutValueLifespan(int(secs*1000.0))

class WireConnectionDirector(RobotRaconteurPython.WrappedWireConnectionDirector):

    def __init__(self,endpoint,type,obj=None,innerep=None):
        self.__endpoint=endpoint
        self.__type=type
        self.__obj=obj
        self.__innerep=innerep
        super(WireConnectionDirector, self).__init__()

    def WireValueChanged(self,value,time):

        value2=UnpackMessageElement(value,self.__type,self.__obj,self.__innerep.GetNode())
        self.__endpoint.WireValueChanged.fire(self.__endpoint,value2,time)


    def WireConnectionClosedCallback(self):

        if (not self.__endpoint.WireConnectionClosedCallback is None):
            self.__endpoint.WireConnectionClosedCallback(self.__endpoint)


class WireAsyncConnectHandlerImpl(RobotRaconteurPython.AsyncWireConnectionReturnDirector):
    def __init__(self,handler,innerpipe,obj):
        super(WireAsyncConnectHandlerImpl,self).__init__()
        self._handler=handler
        self.__innerpipe=innerpipe
        self.__obj=obj

    def handler(self, innerendpoint, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(None,err)
            return
        try:            
            outerendpoint=WireConnection(innerendpoint,self.__innerpipe.Type,self.__obj)
            director=WireConnectionDirector(outerendpoint,self.__innerpipe.Type,self.__obj,innerendpoint)
            innerendpoint.SetRRDirector(director,0)
            director.__disown__()
        except Exception as err2:
            self._handler(None, err2)
            return
        self._handler(outerendpoint, None)

class WireAsyncPeekReturnDirectorImpl(RobotRaconteurPython.AsyncWirePeekReturnDirector):
    def __init__(self,handler,innerpipe,obj):
        super(WireAsyncPeekReturnDirectorImpl,self).__init__()
        self._handler=handler
        self.__innerpipe=innerpipe
        self.__obj=obj

    def handler(self,m,ts,error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler((None, None), err)
            return
        value=UnpackMessageElement(m,self.__innerpipe.Type,self.__obj,self.__innerpipe.GetNode())
        self._handler((value, ts), None)

       
class Wire(object):
    """
    Wire()

    \"wire\" member type interface

    The Wire class implements the \"wire\" member type. Wires are declared in service definition files
    using the \"wire\" keyword within object declarations. Wires provide "most recent" value streaming
    between clients and services. They work by creating "connection" pairs between the client and service.
    The wire streams the current value between the wire connection pairs using packets. Wires 
    are unreliable; only the most recent value is of interest, and any older values 
    will be dropped. Wire connections have an InValue and an OutValue. Users set the OutValue on the
    connection. The new OutValue is transmitted to the peer wire connection, and becomes the peer's
    InValue. The peer can then read the InValue. The client and service have their own InValue
    and OutValue, meaning that each direction, client to service or service to client, has its own
    value.

    Wire connections are created using the Connect() or AsyncConnect() functions. Services receive
    incoming connection requests through a callback function. Thes callback is configured using
    the SetWireConnectCallback() function. Services may also use the WireBroadcaster class
    or WireUnicastReceiver class to automate managing wire connection lifecycles. WireBroadcaster
    is used to send values to all connected clients. WireUnicastReceiver is used to receive the
    value from the most recent wire connection. See WireConnection for details on sending
    and receiving streaming values.

    Wire clients may also optionally "peek" and "poke" the wire without forming a streaming
    connection. This is useful if the client needs to read the InValue or set the OutValue
    instantaniously, but does not need continuous updating. PeekInValue() or 
    AsyncPeekInValue() will retrieve the client's current InValue. PokeOutValue() or
    AsyncPokeOutValue() will send a new client OutValue to the service.
    PeekOutValue() or AsyncPeekOutValue() will retrieve the last client OutValue received by
    the service.

    "Peek" and "poke" operations initiated by the client are received on the service using
    callbacks. Use PeekInValueCallback, PeekOutValueCallback,
    and PokeOutValueCallback to configure the callbacks to handle these requests.
    WireBroadcaster and WireUnicastReceiver configure these callbacks automatically, so 
    the user does not need to configure the callbacks when these classes are used.

    Wires can be declared *readonly* or *writeonly*. If neither is specified, the wire is assumed
    to be full duplex. *readonly* pipes may only send values from service to client, ie OutValue
    on service side and InValue on client side. *writeonly* pipes may only send values from
    client to service, ie OutValue on client side and InValue on service side. Use Direction()
    to determine the direction of the wire.

    Unlike pipes, wire connections are not indexed, so only one connection pair can be
    created per client connection.

    WireBroadcaster or WireUnicastReceiver are typically used to simplify using wires.
    See WireBroadcaster and WireUnicastReceiver for more information.

    This class is instantiated by the node. It should not be instantiated by the user.
    """
    __slots__ = ["_innerpipe", "_obj","__weakref__"]
    def __init__(self,innerpipe,obj=None):
        self._innerpipe=innerpipe        
        self._obj=obj

    def Connect(self):
        """
        Connect the wire

        Creates a connection between the wire, returning the client connection. Used to create
        a "most recent" value streaming connection to the service.

        Only valid on clients. Will throw InvalidOperationException on the service side.

        Note: If a streaming connection is not required, use PeekInValue(), PeekOutValue(),
        or PokeOutValue() instead of creating a connection.

        :return: The wire connection
        :rtype: RobotRaconteur.WireConnection
        """
        innerendpoint=self._innerpipe.Connect()
        outerendpoint=WireConnection(innerendpoint,self._innerpipe.Type,self._obj)
        director=WireConnectionDirector(outerendpoint,self._innerpipe.Type,self._obj,innerendpoint)
        innerendpoint.SetRRDirector(director,0)
        director.__disown__()
        return outerendpoint

    def AsyncConnect(self,handler,timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        """
        Asynchronously connect the wire

        Same as Connect(), but returns asynchronously

        Only valid on clients. Will throw InvalidOperationException on the service side.

        If ``handler`` is None, returns an awaitable future.

        :param handler: A handler function to receive the wire connection, or an exception
        :type handler: Callable[[RobotRaconteur.WireConnection,Exception],None]
        :param timeout: Timeout in seconds, or -1 for infinite
        """
        return async_call(self._innerpipe.AsyncConnect,(adjust_timeout(timeout),),WireAsyncConnectHandlerImpl,handler,directorargs=(self._innerpipe,self._obj))

    @property
    def MemberName(self):
        """
        Get the member name of the wire

        :rtype: str
        """
        return self._innerpipe.GetMemberName()

    @property
    def Direction(self):
        """
        The direction of the wire

        Wires may be declared "readonly" or "writeonly" in the service definition file. (If neither
        is specified, the wire is assumed to be full duplex.) "readonly" wires may only send packets from
        service to client. "writeonly" wires may only send packets from client to service.

        See ``MemberDefinition_Direction`` constants for possible return values.

        :rtype: int
        """
        return self._innerpipe.Direction()

    def AsyncPeekInValue(self, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        """
        Asynchronously peek the current InValue

        Same as PeekInValue(), but returns asynchronously.

        Only valid on clients. Will throw InvalidOperationException on the service side.

        :param handler: A handler function to receive the InValue and timestamp, or an exception
        :type handler: Callable[[T,RobotRaconteur.TimeSpec,Exception],None]
        :param timeout: Timeout in seconds, or -1 for infinite
        :type timeout: float
        """
        return async_call(self._innerpipe.AsyncPeekInValue, (adjust_timeout(timeout),), WireAsyncPeekReturnDirectorImpl, handler, directorargs=(self._innerpipe,self._obj))

    def AsyncPeekOutValue(self, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        """
        Asynchronously peek the current OutValue

        Same as PeekOutValue(), but returns asynchronously.

        Only valid on clients. Will throw InvalidOperationException on the service side.

        :param handler: A handler function to receive the OutValue and timestamp, or an exception
        :type handler: Callable[[T,RobotRaconteur.TimeSpec,Exception],None]
        :param timeout: Timeout in seconds, or -1 for infinite
        :type timeout: float
        """
        return async_call(self._innerpipe.AsyncPeekOutValue, (adjust_timeout(timeout),), WireAsyncPeekReturnDirectorImpl, handler, directorargs=(self._innerpipe,self._obj))

    def AsyncPokeOutValue(self, value, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        """
        Asynchronously poke the OutValue

        Same as PokeOutValue(), but returns asynchronously

        Only valid on clients. Will throw InvalidOperationException on the service side.

        :param handler: A handler function to invoke on completion, with possible exception
        :type handler: Callable[[Exception],None]
        :param value: The new OutValue
        :param timeout: Timeout in seconds, or -1 for no timeout
        :type timeout: float
        """
        m=PackMessageElement(value,self._innerpipe.Type,self._obj,self._innerpipe.GetNode())
        return async_call(self._innerpipe.AsyncPokeOutValue, (m,adjust_timeout(timeout)), AsyncVoidReturnDirectorImpl, handler)

    
class ServiceInfo2(object):
    """
    ServiceInfo2()

    Contains information about a service found using discovery

    ServiceInfo2 contains information about a service required to 
    connect to the service, metadata, and the service attributes

    ServiceInfo2 structures are returned by RobotRaconteurNode.FindServiceByType()
    and ServiceInfo2Subscription
    """
    __slots__ = ["Name", "RootObjectType", "RootObjectImplements", "ConnectionURL", "NodeID", "NodeName", "Attributes"]
    def __init__(self,info):
        self.Name=info.Name
        """(str) The name of the service"""
        self.RootObjectType=info.RootObjectType
        """(str) The fully qualified types the root object implements"""
        self.RootObjectImplements=list(info.RootObjectImplements)
        """(str) The fully qualified types the root object implements"""
        self.ConnectionURL=list(info.ConnectionURL)
        """(List[str]) Candidate URLs to connect to the service"""
        self.NodeID=RobotRaconteurPython.NodeID(str(info.NodeID))
        """(RobotRaconteur.NodeID) The NodeID of the node that owns the service"""
        self.NodeName=info.NodeName
        """(str) The NodeName of the node that owns the service"""
        self.Attributes=UnpackMessageElement(info.Attributes,"varvalue{string} value")
        """(Dict[str,Any]) Service attributes"""

class NodeInfo2(object):
    """
    NodeInfo2()

    Contains information about a node detected using discovery

    NodeInfo2 contains information about a node detected using discovery.
    Node information is typically not verified, and is used as a first
    step to detect available services.

    NodeInfo2 structures are returned by RobotRaconteurNode.FindNodeByName()
    and RobotRaconteurNode.FindNodeByID()
    """
    __slots__ = ["NodeID", "NodeName", "ConnectionURL"]
    def __init__(self,info):
        self.NodeID=RobotRaconteurPython.NodeID(str(info.NodeID))
        """(RobotRaconteur.NodeID) The NodeID of the detected node"""
        self.NodeName=info.NodeName
        """(str) The NodeName of the detected node"""
        self.ConnectionURL=list(info.ConnectionURL)
        """(List[str]) Candidate URLs to connect to the node 

        The URLs for the node typically contain the node transport endpoint
        and the nodeid. A URL service parameter must be appended
        to connect to a service.
        """

class WrappedClientServiceListenerDirector(RobotRaconteurPython.ClientServiceListenerDirector):
    def __init__(self,callback):
        self.callback=callback

        super(WrappedClientServiceListenerDirector,self).__init__()

    def Callback(self,code):

       self.callback(self.stub,code,None)

    def Callback2(self,code,p):

       self.callback(self.stub,code,p)


class RRConstants(object):
    pass

def convert_constant(const):
    
    if not const.VerifyValue():
        raise Exception("Invalid constant " + const.Name)
    t=const.Type
    
    if (t.Type==RobotRaconteurPython.DataTypes_string_t):
        return const.Name, const.ValueToString()

    if RobotRaconteurPython.IsTypeNumeric(t.Type):
        
        if (t.Type==RobotRaconteurPython.DataTypes_double_t or t.Type==RobotRaconteurPython.DataTypes_single_t):
            if (t.ArrayType == RobotRaconteurPython.DataTypes_ArrayTypes_array):
                s3=const.Value.strip().lstrip('{').rstrip('}')
                return const.Name, [float(i) for i in s3.split(',')]
            else:
                return const.Name, float(const.Value)
        else:
            if (t.ArrayType == RobotRaconteurPython.DataTypes_ArrayTypes_array):
                s3=const.Value.strip().lstrip('{').rstrip('}')
                return const.Name, [int(i,0) for i in s3.split(',')]
            else:
                return const.Name, const.Value
    
    if t.Type==RobotRaconteurPython.DataTypes_namedtype_t:
        struct_fields=const.ValueToStructFields()
        f=dict()
        for struct_field in struct_fields:
            f[struct_field.Name]=struct_field.ConstantRefName
        return const.Name, f

    raise Exception("Unknown constant type")

def ServiceDefinitionConstants(servicedef, node, obj):
    o=dict()
    for s in servicedef.Options:
        s2=s.split(None,1)
        if (s2[0]=="constant"):
            c=RobotRaconteurPython.ConstantDefinition(servicedef)
            c.FromString(s)
            name,val=convert_constant(c)
            o[name]=val

    for c in servicedef.Constants:
        name,val=convert_constant(c)
        o[name]=val
    
    elem_o=dict()
    for e in itertools.chain(servicedef.NamedArrays,servicedef.Pods,servicedef.Structures,servicedef.Objects):
        o2=dict()
        for s in e.Options:

            s2=s.split(None,1)
            if (s2[0]=="constant"):
                c=RobotRaconteurPython.ConstantDefinition(servicedef)
                c.FromString(s)
                name,val=convert_constant(c)                
                o2[name]=val
        
        for c in e.Constants:
            name,val=convert_constant(c)
            o2[name]=val
        
        if (len(o2)>0):
            elem_o[e.Name]=o2
    
    for _, c_value in o.items():
        if isinstance(c_value,dict):
            for f_name, f_value in c_value.items():                
                if not f_value in o:
                    raise Exception("Invalid struct reference type: " + f_value)
                c_value[f_name]=o[f_value]
                
                
    for n,v in elem_o.items():
        o[n]=v
    
    for e in servicedef.Enums:
        o_enum=dict()
        for v in e.Values:
            o_enum[v.Name]=v.Value
        o[e.Name] = o_enum
        
    
    return o   

def adjust_timeout(t):
    if (t<0):
        return -1
    else:
        return int(t*1000)

class AsyncStubReturnDirectorImpl(RobotRaconteurPython.AsyncStubReturnDirector):
    def __init__(self,handler):
        super(AsyncStubReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, innerstub2, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(None,err)
            return
        try:            
            stub=innerstub2.GetPyStub()
            if (stub is None):
                stub=InitStub(innerstub2)
                innerstub2.SetPyStub(stub)
        except Exception as err2:
            self._handler(None, err2)
            return
        self._handler(stub, None)


class AsyncVoidReturnDirectorImpl(RobotRaconteurPython.AsyncVoidReturnDirector):
    def __init__(self,handler):
        super(AsyncVoidReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(err)
            return        
        self._handler(None)

class AsyncVoidNoErrReturnDirectorImpl(RobotRaconteurPython.AsyncVoidNoErrReturnDirector):
    def __init__(self,handler):
        super(AsyncVoidNoErrReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self):
        self._handler()

class AsyncStringReturnDirectorImpl(RobotRaconteurPython.AsyncStringReturnDirector):
    def __init__(self,handler):
        super(AsyncStringReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, istr, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(None,err)
            return       
        self._handler(istr, None)

class AsyncUInt32ReturnDirectorImpl(RobotRaconteurPython.AsyncUInt32ReturnDirector):
    def __init__(self,handler):
        super(AsyncUInt32ReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, e, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(None,err)
            return       
        self._handler(e, None)

class AsyncTimerEventReturnDirectorImpl(RobotRaconteurPython.AsyncTimerEventReturnDirector):
    def __init__(self,handler):
        super(AsyncTimerEventReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, ev, error_info):        
        self._handler(ev)

def async_call(func, args, directorclass, handler, noerror=False, directorargs=()):
    d=None
    if (handler is None):
        if (sys.version_info > (3,5)):
            d = WebFuture()
                                    
            def handler3(*args):
                if noerror:
                    if len(args) == 0:
                        d.handler(None,None)
                    else:
                        d.handler(args[0],None)
                else:
                    ret = None
                    if len(args) == 2:
                        ret = args[0]
                    if args[-1] is None:
                        d.handler(ret,None)
                    else:
                        d.handler(None,args[-1])
            handler = lambda *args1: handler3(*args1)            
        else:
            raise Exception("handler must not be None")
            
    handler2=directorclass(handler,*directorargs)
    args2=list(args)
    args2.extend([handler2,0])
    handler2.__disown__()
    func(*args2)
    return d

class ExceptionHandlerDirectorImpl(RobotRaconteurPython.AsyncVoidReturnDirector):
    def __init__(self,handler):
        super(ExceptionHandlerDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(err)
            return

class GeneratorClient(object):
    __slots__ = ["_inner_gen", "_obj", "_node", "_return_type", "_param_type","__weakref__"]
    def __init__(self, inner_gen, return_type, param_type, obj, node):
        self._inner_gen=inner_gen
        self._obj=obj
        self._node=node        
        self._return_type = return_type
        self._param_type = param_type
    
    def _pack_param(self, param):
        param_type1=RobotRaconteurPython.TypeDefinition()
        self._param_type.CopyTo(param_type1)
        param_type1.RemoveContainers()            
        param1 = PackMessageElement(param, param_type1, self._obj, self._node)
        return param1
    
    def _unpack_return(self, ret):
        return_type1=RobotRaconteurPython.TypeDefinition()
        self._return_type.CopyTo(return_type1)
        return_type1.RemoveContainers()
        return UnpackMessageElement(ret,return_type1, self._obj, self._node)  
    
    def AsyncNext(self, param, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        if (self._param_type is not None and self._param_type.ContainerType == DataTypes_ContainerTypes_generator):
            param1 = self._pack_param(param)
        else:
            assert param is None
            param1 = None    
        
        return_type1=RobotRaconteurPython.TypeDefinition()
        self._return_type.CopyTo(return_type1)
        return_type1.RemoveContainers()
        
        if (self._return_type.Type != RobotRaconteurPython.DataTypes_void_t):
            return async_call(self._inner_gen.AsyncNext,(param1,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(False,return_type1,self._obj,self._node))
        else:
            return async_call(self._inner_gen.AsyncNext,(param1,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(True,return_type1,self._obj,self._node))
    
    def AsyncAbort(self, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        return async_call(self._inner_gen.AsyncAbort,(adjust_timeout(timeout),),AsyncVoidReturnDirectorImpl,handler)
    
    def AsyncClose(self, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        return async_call(self._inner_gen.AsyncClose,(adjust_timeout(timeout),),AsyncVoidReturnDirectorImpl,handler)           
    
    #Add compatibility for iterator protocols
    def __iter__(self):
        if self._return_type is None or self._param_type is not None:
            raise TypeError('Generator must be type 1 for iterable')
        return self


class AsyncGeneratorClientReturnDirectorImpl(RobotRaconteurPython.AsyncGeneratorClientReturnDirector):
    def __init__(self, handler, return_type, param_type, obj, node):
        super(AsyncGeneratorClientReturnDirectorImpl,self).__init__()
        self._handler=handler
        self._return_type=return_type
        self._param_type=param_type
        self._obj=obj
        self._node=node
        
    def handler(self, gen, error_info):
        if (error_info.error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)
            self._handler(None,err)
            return
        
        gen2=GeneratorClient(gen, self._return_type, self._param_type, self._obj, self._node)
        self._handler(gen2, None)

       
_trace_hook=sys.gettrace()

class ServiceSubscriptionClientID(object):
    """
    ServiceSubscriptionClientID()
    
    ClientID for use with ServiceSubscription

    The ServiceSubscriptionClientID stores the NodeID
    and ServiceName of a connected service.
    """
    __slots__ = ["NodeID", "ServiceName"]
    def __init__(self, *args):
        self.NodeID=None
        """(RobotRaconteur.NodeID) The NodeID of the connected service"""
        self.ServiceName=None
        """(str) The ServiceName of the connected service"""

        if (len(args) == 1):
            self.NodeID=args[0].NodeID
            self.ServiceName=args[0].ServiceName
        elif (len(args) == 2):
            self.NodeID=args[0];
            self.ServiceName=args[1];

    def __eq__(self, other):
        if (not hasattr(other,'NodeID') or not hasattr(other,'ServiceName')):
            return False;
        return (self.NodeID == other.NodeID) and (self.ServiceName == other.ServiceName)

    def __neq__(self,other):
        return not self == other

    def __hash__(self):
      return hash((str(self.NodeID), self.ServiceName))

class ServiceSubscriptionFilterNode(object):
    """
    Subscription filter node information

    Specify a node by NodeID and/or NodeName. Also allows specifying
    username and password.

    When using username and credentials, secure transports and specified NodeID should
    be used. Using username and credentials without a transport that verifies the
    NodeID could result in credentials being leaked.
    """
    __slots__ = ["NodeID", "NodeName", "Username", "Credentials"]
    def __init__(self):
        self.NodeID=None
        """(RobotRaconteur.NodeID) The NodeID to match. All zero NodeID will match any NodeID."""
        self.NodeName=None
        """(str) The NodeName to match. Emtpy NodeName will match any NodeName."""
        self.Username=None
        """(str) The username to use for authentication. Should only be used with secure transports and verified NodeID"""
        self.Credentials=None
        """(Dict[str,Any]) The credentials to use for authentication. Should only be used with secure transports and verified NodeID"""

class ServiceSubscriptionFilter(object):
    """
    Subscription filter

    The subscription filter is used with RobotRaconteurNode.SubscribeServiceByType() and 
    RobotRaconteurNode.SubscribeServiceInfo2() to decide which services should
    be connected. Detected services that match the service type are checked against
    the filter before connecting.
    """
    __slots__ = ["Nodes", "ServiceNames", "TransportSchemes", "Predicate", "MaxConnections"]
    def __init__(self):
        self.Nodes=[]
        """(List[RobotRaconteurServiceSubscriptionFilterNode]) List of nodes that should be connected. Empty means match any node."""
        self.ServiceNames=[]
        """(List[str])  List of service names that should be connected. Empty means match any service name."""
        self.TransportSchemes=[]
        """(List[str]) List of transport schemes. Empty means match any transport scheme."""
        self.Predicate=None
        """(Callable[[RobotRaconteur.ServiceInfo2],bool]) A user specified predicate function. If nullptr, the predicate is not checked."""
        self.MaxConnections=1000000
        """(int) The maximum number of connections the subscription will create. Zero means unlimited connections."""

class WrappedServiceInfo2SubscriptionDirectorPython(RobotRaconteurPython.WrappedServiceInfo2SubscriptionDirector):
    def __init__(self, subscription):
        super(WrappedServiceInfo2SubscriptionDirectorPython,self).__init__()
        self._subscription=weakref.ref(subscription);

    def ServiceDetected(self, subscription, id, info):
        s = self._subscription()
        if (s is None):
            return       
        try:
            s.ServiceDetected.fire(s, ServiceSubscriptionClientID(id), ServiceInfo2(info))
        except: pass

    def ServiceLost(self, subscription, id, info):
        s = self._subscription()
        if (s is None):
            return        
        try:
            s.ServiceLost.fire(s, ServiceSubscriptionClientID(id), ServiceInfo2(info))
        except: pass

class ServiceInfo2Subscription(object):
    """
    ServiceInfo2Subscription()

    Subscription for information about detected services 

    Created using RobotRaconteurNode.SubscribeServiceInfo2()

    The ServiceInfo2Subscription class is used to track services with a specific service type as they are
    detected on the local network and when they are lost. The currently detected services can also
    be retrieved. The service information is returned using the ServiceInfo2 structure.
    """
    __slots__ = ["_subscription", "_ServiceDetected", "_ServiceLost","__weakref__"]
    def __init__(self, subscription):
        self._subscription=subscription        
        self._ServiceDetected=EventHook()
        self._ServiceLost=EventHook()
        director=WrappedServiceInfo2SubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
    
    
    def GetDetectedServiceInfo2(self):
        """
        Returns a dict of detected services.

        The returned dict contains the detected nodes as ServiceInfo2. The dict
        is keyed with ServiceSubscriptionClientID.

        This function does not block.

        :return: The detected services.
        :rtype: Dict[RobotRaconteur.ServiceSubscriptionClientID,ServiceInfo2]
        """
        o=dict()
        c1=self._subscription.GetDetectedServiceInfo2()
        for c2 in c1.items():
            id1=ServiceSubscriptionClientID(c2[0])
            stub=ServiceInfo2(c2[1])
            o[id1]=stub        
        return o

    def Close(self):
        """
        Close the subscription

        Closes the subscription. Subscriptions are automatically closed when the node is shut down.
        """
        self._subscription.Close()
            
    @property
    def ServiceDetected(self):
        """
        Event hook for service detected events. Use to add handlers to be called
        when a service is detected.

        .. code-block:: python

           def my_handler(sub, subscription_id, service_info2):
              # Process detected service
              pass              

           my_serviceinfo2_sub.ServiceDetected += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.ServiceInfo2Subscription,RobotRaconteur.ServiceSubscriptionClientID,RobotRaconteur.ServiceInfo2],None]``

        :rtype: RobotRaconteur.EventHook
        """
        return self._ServiceDetected
    
    @ServiceDetected.setter
    def ServiceDetected(self, evt):
        if (evt is not self._ServiceDetected):
            raise RuntimeError("Invalid operation")
    
    @property
    def ServiceLost(self):
        """
        Event hook for service lost events. Use to add handlers to be called
        when a service is lost.

        .. code-block:: python

           def my_handler(sub, subscription_id, service_info2):
              # Process lost service
              pass              

           my_serviceinfo2_sub.ServiceLost += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.ServiceInfo2Subscription,RobotRaconteur.ServiceSubscriptionClientID,RobotRaconteur.ServiceInfo2],None]``

        :rtype: RobotRaconteur.EventHook
        """
        return self._ServiceLost
    
    @ServiceLost.setter
    def ServiceLost(self, evt):
        if (evt is not self._ServiceLost):
            raise RuntimeError("Invalid operation")

class WrappedServiceSubscriptionDirectorPython(RobotRaconteurPython.WrappedServiceSubscriptionDirector):
    def __init__(self, subscription):
        super(WrappedServiceSubscriptionDirectorPython,self).__init__()
        self._subscription=weakref.ref(subscription);

    def ClientConnected(self, subscription, id, client):
        s = self._subscription()
        if (s is None):
            return
        
        client2=s._GetClientStub(client)
        
        try:
            s.ClientConnected.fire(s, ServiceSubscriptionClientID(id), client2)
        except:
            traceback.print_exc()

    def ClientDisconnected(self, subscription, id, client):
        s = self._subscription()
        if (s is None):
            return

        client2=s._GetClientStub(client)

        try:
            s.ClientDisconnected.fire(s, ServiceSubscriptionClientID(id), client2)
        except:
            traceback.print_exc()

    def ClientConnectFailed(self, subscription, id, url, error_info):
        s = self._subscription()
        if (s is None):
            return

        err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorInfoToException(error_info)

        try:
            s.ClientConnectFailed.fire(s, ServiceSubscriptionClientID(id), list(url), err)
        except:
            traceback.print_exc()

class ServiceSubscription(object):
    """
    ServiceSubscription()

    Subscription that automatically connects services and manages lifecycle of connected services

    Created using RobotRaconteurNode.SubscribeService() or RobotRaconteurNode.SubscribeServiceByType(). The ServiceSubscription
    class is used to automatically create and manage connections based on connection criteria. RobotRaconteur.SubscribeService()
    is used to create a robust connection to a service with a specific URL. RobotRaconteurNode.SubscribeServiceByType() is used
    to connect to services with a specified type, filtered with a ServiceSubscriptionFilter. Subscriptions will create connections
    to matching services, and will retry the connection if it fails or the connection is lost. This behavior allows subscriptions
    to be used to create robust connections. The retry delay for connections can be modified using the ConnectRetryDelay property.

    The currently connected clients can be retrieved using the GetConnectedClients() function. A single "default client" can be
    retrieved using the GetDefaultClient() function or TryGetDefaultClient() functions. Listeners for client connect and 
    disconnect events can be added using the ClientConnectListener and ClientDisconnectListener properties. If 
    the user wants to claim a client, the ClaimClient() and ReleaseClient() functions will be used. Claimed clients will 
    no longer have their lifecycle managed by the subscription.

    Subscriptions can be used to create \"pipe\" and \"wire\" subscriptions. These member subscriptions aggregate
    the packets and values being received from all services. They can also act as a "reverse broadcaster" to 
    send packets and values to all services that are actively connected. See PipeSubscription and WireSubscription.
    """
    def __init__(self, subscription):
        self._subscription=subscription        
        self._ClientConnected=EventHook()
        self._ClientDisconnected=EventHook()
        self._ClientConnectFailed=EventHook()
        director=WrappedServiceSubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
    
    def _GetClientStub(self, innerstub):
        if (innerstub is None):
            return None
        s2=innerstub.GetPyStub()
        if (s2 is not None): return s2
        return InitStub(innerstub)
        
    def GetConnectedClients(self):
        """
        Returns a dict of connected clients

        The returned dict contains the connect clients. The dict
        is keyed with ServiceSubscriptionClientID.

        Clients must be cast to a type, similar to the client returned by
        RobotRaconteurNode.ConnectService().

        Clients can be "claimed" using ClaimClient(). Once claimed, the subscription
        will stop managing the lifecycle of the client.

        This function does not block.

        :rtype: Dict[RobotRaconteur.ServiceSubscriptionClientID,Any]
        """
        o=dict()
        c1=self._subscription.GetConnectedClients()
        for c2 in c1.items():
            id1=ServiceSubscriptionClientID(c2[0])
            stub=self._GetClientStub(c2[1])
            o[id1]=stub        
        return o

    def Close(self):
        """
        Close the subscription

        Closes the subscription. Subscriptions are automatically closed when the node is shut down.
        """
        self._subscription.Close()

    @property
    def ConnectRetryDelay(self):
        """
        Set the connect retry delay in seconds

        Default is 2.5 seconds

        :rtype: float
        """
        return self._subscription.GetConnectRetryDelay() / 1000.0

    @ConnectRetryDelay.setter
    def ConnectRetryDelay(self,value):
        if (value < 1):
            raise Exception("Invalid ConnectRetryDelay value")
        self._subscription.SetConnectRetryDelay(int(value*1000.0))

    def SubscribeWire(self, wire_name, service_path = None):
        """
        Creates a wire subscription

        Wire subscriptions aggregate the value received from the connected services. It can also act as a 
        "reverse broadcaster" to send values to clients. See WireSubscription.

        The optional service path may be an empty string to use the root object in the service. The first level of the
        service path may be "*" to match any service name. For instance, the service path "*.sub_obj" will match
        any service name, and use the "sub_obj" objref.

        :param membername: The member name of the wire
        :type membername: str
        :param servicepath: The service path of the object owning the wire member
        :type servicepath: str
        :return: The wire subscription
        :rtype: RobotRaconteur.WireSubscription
        """
        if service_path is None:
            service_path = ""
        s=self._subscription.SubscribeWire(wire_name, service_path)
        return WireSubscription(s)

    def SubscribePipe(self, pipe_name, service_path = None):
        """
        Creates a pipe subscription

        Pipe subscriptions aggregate the packets received from the connected services. It can also act as a 
        "reverse broadcaster" to send packets to clients. See PipeSubscription.

        The optional service path may be an empty string to use the root object in the service. The first level of the
        service path may be "*" to match any service name. For instance, the service path "*.sub_obj" will match
        any service name, and use the "sub_obj" objref.

        :param membername: The member name of the pipe
        :type membername: str
        :param servicepath: The service path of the object owning the pipe member
        :type servicepath: str
        :return: The pipe subscription
        :rtype: RobotRaconteur.PipeSubscription
        """
        if service_path is None:
            service_path = ""
        s=self._subscription.SubscribePipe(pipe_name, service_path)
        return PipeSubscription(s)
    
    @property
    def ClientConnected(self):
        """
        Event hook for client connected events. Use to add handlers to be called
        when a client is connected.

        .. code-block:: python

           def my_handler(sub, subscription_id, connected_service):
              # Process lost service
              pass              

           my_service_sub.ClientConnected += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.ServiceInfo2Subscription,RobotRaconteur.ServiceSubscriptionClientID,T],None]``

        :rtype: RobotRaconteur.EventHook
        """
        return self._ClientConnected
    
    @ClientConnected.setter
    def ClientConnected(self, evt):
        if (evt is not self._ClientConnected):
            raise RuntimeError("Invalid operation")
    
    @property
    def ClientDisconnected(self):
        """
        Event hook for client disconnected events. Use to add handlers to be called
        when a client is disconnected.

        .. code-block:: python

           def my_handler(sub, subscription_id, connected_service):
              # Process lost service
              pass              

           my_service_sub.ClientDisconnected += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.ServiceInfo2Subscription,RobotRaconteur.ServiceSubscriptionClientID,T],None]``

        :rtype: RobotRaconteur.EventHook
        """
        return self._ClientDisconnected
    
    @ClientDisconnected.setter
    def ClientDisconnected(self, evt):
        if (evt is not self._ClientDisconnected):
            raise RuntimeError("Invalid operation")

    @property
    def ClientConnectFailed(self):
        """
        Event hook for client client connect failed events. Used to receive
        notification of when a client connection was not successful, including
        the urls and resulting exceptions.

        .. code-block:: python

           def my_handler(sub, subscription_id, candidate_urls, exceptions):
              # Process lost service
              pass              

           my_service_sub.ClientDisconnected += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.ServiceInfo2Subscription,RobotRaconteur.ServiceSubscriptionClientID,List[str],List[Exception]],None]``

        :rtype: RobotRaconteur.EventHook
        """
        return self._ClientConnectFailed
    
    @ClientConnectFailed.setter
    def ClientConnectFailed(self, evt):
        if (evt is not self._ClientConnectFailed):
            raise RuntimeError("Invalid operation")

    def GetDefaultClient(self):
        """
        Get the "default client" connection

        The "default client" is the "first" client returned from the connected clients map. This is effectively
        default, and is only useful if only a single client connection is expected. This is normally true
        for RobotRaconteurNode.SubscribeService()

        Clients using GetDefaultClient() should not store a reference to the client. It should instead
        call GetDefaultClient() right before using the client to make sure the most recenty connection
        is being used. If possible, SubscribePipe() or SubscribeWire() should be used so the lifecycle
        of pipes and wires can be managed automatically.

        :return: The client connection
        """
        return self._GetClientStub(self._subscription.GetDefaultClient())

    def TryGetDefaultClient(self):
        """
        Try getting the "default client" connection

        Same as GetDefaultClient(), but returns a bool success instead of throwing
        exceptions on failure.

        :return: Success and client (if successful) as a tuple
        :rtype: Tuple[bool,T]
        """
        res = self._subscription.TryGetDefaultClient()
        if not res.res:
            return False, None
        return True, self._GetClientStub(res.client)

    def AsyncGetDefaultClient(self, handler, timeout=-1):
        """
        Asynchronously get the default client, with optional timeout

        Same as GetDefaultClientWait(), but returns asynchronously.

        If ``handler`` is None, returns an awaitable future.

        :param handler: The handler to call when default client is available, or times out
        :type handler: Callable[[bool,object],None]
        :param timeout: Timeout in seconds, or -1 for infinite
        :type timeout: float
        """
        return async_call(self._subscription.AsyncGetDefaultClient, (adjust_timeout(timeout),), AsyncStubReturnDirectorImpl, handler)
            
class WrappedWireSubscriptionDirectorPython(RobotRaconteurPython.WrappedWireSubscriptionDirector):
    def __init__(self,subscription):
        super(WrappedWireSubscriptionDirectorPython,self).__init__()
        self._subscription=weakref.ref(subscription)

    def WireValueChanged(self, subscription, value, time):
        s=self._subscription()
        if (s is None):
            return

        try:            
            v=RobotRaconteurPython._UnpackMessageElement(value.packet, value.type, value.stub, None)            
            s.WireValueChanged.fire(s,v,time)
        except:
            traceback.print_exc()

class WireSubscription(object):
    """
    WireSubscription()

    Subscription for wire members that aggregates the values from client wire connections

    Wire subscriptions are created using the ServiceSubscription.SubscribeWire() function. This function takes the
    type of the wire value, the name of the wire member, and an optional service path of the service
    object that owns the wire member.

    Wire subscriptions aggregate the InValue from all active wire connections. When a client connects,
    the wire subscriptions will automatically create wire connections to the wire member specified
    when the WireSubscription was created using ServiceSubscription.SubscribeWire(). The InValue of
    all the active wire connections are collected, and the most recent one is used as the current InValue
    of the wire subscription. The current value, the timespec, and the wire connection can be accessed
    using GetInValue() or TryGetInValue().

    The lifespan of the InValue can be configured using the InValueLifeSpan property. It is recommended that
    the lifespan be configured, so that the value will expire if the subscription stops receiving
    fresh in values.

    The wire subscription can also be used to set the OutValue of all active wire connections. This behaves
    similar to a "reverse broadcaster", sending the same value to all connected services.
    """
    __slots__ = ["_subscription", "_WireValueChanged","__weakref__"]
    def __init__(self, subscription):
        self._subscription=subscription
        director=WrappedWireSubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
        self._WireValueChanged=EventHook()

    def _UnpackValue(self, m):
        return RobotRaconteurPython._UnpackMessageElement(m.packet, m.type, m.stub, None)

    @property
    def InValue(self):
        """
        Get the current InValue

        Throws ValueNotSetException if no valid value is available

        :return: The current InValue
        """
        return self._UnpackValue(self._subscription.GetInValue())

    @property
    def InValueWithTimeSpec(self):
        """
        Get the current InValue and TimeSpec

        Throws ValueNotSetException if no valid value is available

        :return: The current InValue and TimeSpec
        :rtype: Tuple[T,RobotRaconteur.TimeSpec]
        """
        t=RobotRaconteurPython.TimeSpec()
        m=self._subscription.GetInValue(t)
        return (self._UnpackValue(m), t)


    def TryGetInValue(self):
        """
        Try getting the current InValue and metadata

        Same as GetInValue(), but returns a bool for success or failure instead of throwing
        an exception.

        :return: Success and value (if successful)
        :rtype: Tuple[bool,T]
        """
        val = RobotRaconteurPython.WrappedService_typed_packet()
        t=RobotRaconteurPython.TimeSpec()
        res=self._subscription.TryGetInValue(val,t)
        if not res:
            return False, None, None
        return (True, self._UnpackValue(val), t)

    @property
    def ActiveWireConnectionCount(self):
        """
        Get the number of wire connections currently connected
        
        :rtype: int
        """
        return self._subscription.GetActiveWireConnectionCount()

    @property
    def IgnoreInValue(self):
        """
        Set if InValue should be ignored

        See WireConnection.IgnoreInValue

        If true, InValue will be ignored for all wire connections

        :rtype: bool
        """
        return self._subscription.GetIgnoreInValue()

    @IgnoreInValue.setter
    def IgnoreInValue(self, ignore):
        self._subscription.SetIgnoreInValue(ignore)

    @property
    def InValueLifespan(self):
        """
        Set the InValue lifespan in seconds

        Set the lifespan of InValue in seconds. The value will expire after
        the specified lifespan, becoming invalid. Use -1 for infinite lifespan.

        See also WireConnection.InValueLifespan

        :rtype: float
        """
        t = self._subscription.GetInValueLifespan()
        if t < 0:
            return t
        return float(t) / 1000.0

    @InValueLifespan.setter
    def InValueLifespan(self, secs):
        if secs < 0:
            self._subscription.SetInValueLifespan(-1)
        else:
            self._subscription.SetInValueLifespan(int(secs*1000.0))

    def SetOutValueAll(self, value):
        """
        Set the OutValue for all active wire connections

        Behaves like a "reverse broadcaster". Calls WireConnection.OutValue
        for all connected wire connections.

        :param value: The value to send
        """      
        iter=RobotRaconteurPython.WrappedWireSubscription_send_iterator(self._subscription)
        try:
            while iter.Next() is not None:
                m=PackMessageElement(value,iter.GetType(),iter.GetStub())
                iter.SetOutValue(m)                
        finally:
            del iter

    def Close(self):
        """
        Closes the wire subscription

        Wire subscriptions are automatically closed when the parent ServiceSubscription is closed
        or when the node is shut down.
        """
        self._subscription.Close()
        
    @property
    def WireValueChanged(self):
        """
        Event hook for wire value change. Use to add handlers to be called
        when the InValue changes.

        .. code-block:: python

           def my_handler(sub, value, ts):
              # Handle new value
              pass              

           my_wire_csub.WireValueChanged += my_handler

        Handler must have signature ``Callable[[RobotRaconteur.WireSubscription,T,RobotRaconteur.TimeSpec],None]``

        :rtype: RobotRaconteur.EventHook

        """
        return self._WireValueChanged
    
    @WireValueChanged.setter
    def WireValueChanged(self, evt):
        if (evt is not self._WireValueChanged):
            raise RuntimeError("Invalid operation")

class WrappedPipeSubscriptionDirectorPython(RobotRaconteurPython.WrappedPipeSubscriptionDirector):
    def __init__(self,subscription):
        super(WrappedPipeSubscriptionDirectorPython,self).__init__()
        self._subscription=weakref.ref(subscription)

    def PipePacketReceived(self, subscription):
        s=self._subscription()
        if (s is None):
            return

        try:            
            s.PipePacketReceived.fire(s)
        except:
            traceback.print_exc()

class PipeSubscription(object):
    """
    PipeSubscription()

    Subscription for pipe members that aggregates incoming packets from client pipe endpoints

    Pipe subscriptions are created using the ServiceSubscription.SubscribePipe() function. This function takes the
    the type of the pipe packets, the name of the pipe member, and an optional service path of the service
    object that owns the pipe member.

    Pipe subscriptions collect all incoming packets from connect pipe endpoints. When a client connects,
    the pipe subscription will automatically connect a pipe endpoint the pipe endpoint specified when
    the PipeSubscription was created using ServiceSubscription.SubscribePipe(). The packets received
    from each of the collected pipes are collected and placed into a common receive queue. This queue
    is read using ReceivePacket(), TryReceivePacket(), or TryReceivePacketWait(). The number of packets
    available to receive can be checked using Available().

    Pipe subscriptions can also be used to send packets to all connected pipe endpoints. This is done
    with the AsyncSendPacketAll() function. This function behaves somewhat like a "reverse broadcaster",
    sending the packets to all connected services.

    If the pipe subscription is being used to send packets but not receive them, IgnoreInValue
    should be set to true to prevent packets from queueing.
    """
    __slots__ = ["_subscription", "_PipePacketReceived","__weakref__"]
    def __init__(self, subscription):
        self._subscription=subscription
        director=WrappedPipeSubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
        self._PipePacketReceived=EventHook()

    def _UnpackValue(self, m):
        return RobotRaconteurPython._UnpackMessageElement(m.packet, m.type, m.stub, None)

    def ReceivePacket(self):
        """
        Dequeue a packet from the receive queue

        If the receive queue is empty, an InvalidOperationException() is thrown

        :return: The dequeued packet
        """
        return self._UnpackValue(self._subscription.ReceivePacket())

    def TryReceivePacket(self):
        """
        Try dequeuing a packet from the receive queue

        Same as ReceivePacket(), but returns a bool for success or failure instead of throwing
        an exception

        :return: Success and packet (if successful)
        :rtype: Tuple[bool,T]
        """
        return self.TryReceivePacketWait(0)

    def TryReceivePacketWait(self, timeout=-1,peek=False):
        """
        Try dequeuing a packet from the receive queue, optionally waiting or peeking the packet

        :param timeout: The time to wait for a packet to be received in seconds if the queue is empty, or -1 to wait forever
        :type timeout: float
        :param peek: If True, the packet is returned, but not dequeued. If False, the packet is dequeued
        :type peek: bool
        :return: Success and packet (if successful)
        :rtype: Tuple[bool,T]
        """
        val = RobotRaconteurPython.WrappedService_typed_packet()
        res=self._subscription.TryReceivePacketWait(val,adjust_timeout(timeout),peek)
        if (not res):
            return (False, None)
        else:
            return (True, self._UnpackValue(val))

    @property
    def Available(self):
        """
        Get the number of packets available to receive

        Use ReceivePacket(), TryReceivePacket(), or TryReceivePacketWait() to receive the packet

        :rtype: int
        """
        return self._subscription.Available()

    def AsyncSendPacketAll(self, packet):
        """
        Sends a packet to all connected pipe endpoints

        Calls AsyncSendPacket() on all connected pipe endpoints with the specified value.
        Returns immediately, not waiting for transmission to complete.

        :param packet: The packet to send
        """        
        iter=RobotRaconteurPython.WrappedPipeSubscription_send_iterator(self._subscription)
        try:
            while iter.Next() is not None:
                m=PackMessageElement(packet,iter.GetType(),iter.GetStub())
                iter.AsyncSendPacket(m)                
        finally:
            del iter

    @property
    def ActivePipeEndpointCount(self):
        """
        Get the number of pipe endpoints currently connected

        :rtype: int
        """
        return self._subscription.GetActivePipeEndpointCount()

    def Close(self):
        """
        Closes the pipe subscription

        Pipe subscriptions are automatically closed when the parent ServiceSubscription is closed
        or when the node is shut down.
        """
        return self._subscription.Close()
    
    @property
    def PipePacketReceived(self):
        """
        Event hook for packet received. Use to add handlers to be called
        when the subscription receives a new packet.

        .. code-block:: python

           def my_handler(pipe_sub):
              while pipe_sub.Available > 0:
                  packet = pipe_sub.ReceivePacket()
                  # Handle new packet
                  pass              

           my_pipe_sub.PipePacketReceived+= my_handler

        Handler must have signature ``Callable[[RobotRaconteur.PipeSubscription],None]``

        :rtype: RobotRaconteur.EventHook

        """
        return self._PipePacketReceived
    
    @PipePacketReceived.setter
    def PipePacketReceived(self, evt):
        if (evt is not self._PipePacketReceived):
            raise RuntimeError("Invalid operation")   

class WrappedServiceSubscriptionFilterPredicateDirectorPython(RobotRaconteurPython.WrappedServiceSubscriptionFilterPredicateDirector):
    def __init__(self, f):
        super(WrappedServiceSubscriptionFilterPredicateDirectorPython,self).__init__()
        self._f=f

    def Predicate(self, info):
        info2=ServiceInfo2(info)
        return self._f(info2)

def _SubscribeService_LoadFilter(node, filter_):
    filter2=None
    if (filter_ is not None):
        filter2=RobotRaconteurPython.WrappedServiceSubscriptionFilter()
        if (filter_.ServiceNames is not None):
            for s in filter_.ServiceNames:
                filter2.ServiceNames.append(s)
        if (filter_.TransportSchemes is not None):
            for s in filter_.TransportSchemes:
                filter2.TransportSchemes.append(s)
        filter2.MaxConnections=filter_.MaxConnections

        if (filter_.Nodes is not None):
            nodes2=RobotRaconteurPython.vectorptr_wrappedservicesubscriptionnode()
            for n1 in filter_.Nodes:
                if (n1 is None):
                    continue
                n2=RobotRaconteurPython.WrappedServiceSubscriptionFilterNode()
                if (n1.NodeID is not None): n2.NodeID=n1.NodeID
                if (n1.NodeName is not None): n2.NodeName=n1.NodeName
                if (n1.Username is not None): n2.Username=n1.Username
                if (n1.Credentials is not None):
                    n2.Credentials=PackMessageElement(n1.Credentials,"varvalue{string}",None,node).GetData()
                
                nodes2.append(n2)
            filter2.Nodes=nodes2

        if (filter_.Predicate is not None):
            director=WrappedServiceSubscriptionFilterPredicateDirectorPython(filter_.Predicate)
            filter2.SetRRPredicateDirector(director, 0)
            director.__disown__()
    return filter2

def SubscribeServiceInfo2(node, service_types, filter_=None):

    filter2=_SubscribeService_LoadFilter(node, filter_)

    service_types2=RobotRaconteurPython.vectorstring()
    if (sys.version_info  > (3,0)):
        if (isinstance(service_types, str)):
            service_types2.append(service_types)
        else:
            for s in service_types: service_types2.append(s)
    else:
        if (isinstance(service_types, (str, unicode))):
            service_types2.append(service_types)
        else:
            for s in service_types: service_types2.append(s)

    
    sub1=RobotRaconteurPython.WrappedSubscribeServiceInfo2(node, service_types2, filter2)
    return ServiceInfo2Subscription(sub1)

def SubscribeServiceByType(node, service_types, filter_=None):

    filter2=_SubscribeService_LoadFilter(node, filter_)

    service_types2=RobotRaconteurPython.vectorstring()
    if (sys.version_info  > (3,0)):
        if (isinstance(service_types, str)):
            service_types2.append(service_types)
        else:
            for s in service_types: service_types2.append(s)
    else:
        if (isinstance(service_types, (str, unicode))):
            service_types2.append(service_types)
        else:
            for s in service_types: service_types2.append(s)

    
    sub1=RobotRaconteurPython.WrappedSubscribeServiceByType(node, service_types2, filter2)
    return ServiceSubscription(sub1)

def SubscribeService(node, *args):
    args2=list(args)
    if (len(args) >= 3):
        if (args[1]==None): args2[1]=""
        args2[2]=PackMessageElement(args[2],"varvalue{string}",None,node).GetData()    
    sub1=RobotRaconteurPython.WrappedSubscribeService(node, *args2)
    return ServiceSubscription(sub1)   

        

def ReadServiceDefinitionFile(servicedef_name):
    f_name = None
    if (os.path.isfile(servicedef_name)):
        f_name=servicedef_name
    elif(os.path.isfile(servicedef_name + '.robdef')):
        f_name=servicedef_name + '.robdef'
    elif not os.path.isabs(servicedef_name):
        p = os.getenv("ROBOTRACONTEUR_ROBDEF_PATH", None)
        if p is not None:
            p1=p.split(os.pathsep)
            for p2 in p1:
                p3=p2.strip()
                if (os.path.isfile(os.path.join(p3, servicedef_name))):
                    f_name=os.path.join(p3, servicedef_name)
                if (os.path.isfile(os.path.join(p3, servicedef_name + '.robdef'))):
                    f_name=os.path.join(p3, servicedef_name + '.robdef')
    
    if f_name is None:
        raise IOError("Service definition file %s not found" % servicedef_name)
    
    with codecs.open(f_name, 'r', 'utf-8-sig') as f:
        return f.read()

def ReadServiceDefinitionFiles(servicedef_names, auto_import = False):

    d = []
    for servicedef_name in servicedef_names:
        d1 = RobotRaconteurPython.ServiceDefinition()
        d1.FromString(str(ReadServiceDefinitionFile(servicedef_name)))
        d.append(d1)
    
    if auto_import:
        missing_imports = set()
        d2 = {d3.Name:d3 for d3 in d}
        for k,v in d2.items():
            for imported in v.Imports:
                if imported not in d2:
                    missing_imports.add(imported)

        attempted_imports = set()
        while len(missing_imports) != 0:
            e = missing_imports.pop()
            d1 = RobotRaconteurPython.ServiceDefinition()            
            d1.FromString(str(ReadServiceDefinitionFile(e)))
            d.append(d1)
            d2[d1.Name] = d1
            for imported in d1.Imports:
                if imported not in d2:
                    missing_imports.add(imported)
    return d

class RobotRaconteurNodeSetup(object):
    """
    Setup a node using specified options and manage node lifecycle

    RobotRaconteurNodeSetup and its subclasses ClientNodeSetup, ServerNodeSetup,
    and SecureServerNodeSetup are designed to help configure nodes and manage
    node lifecycles. The node setup classes use the "with" statement to configure the node
    on construction, and call RobotRaconteurNode.Shutdown() when the instance
    is destroyed.

    The node setup classes execute the following operations to configure the node:

    1. Set log level and tap options from flags, command line options, or environmental variables
    2. Register specified service factory types
    3. Initialize transports using flags specified in flags or from command line options
    4. Configure timeouts

    See Command Line Options for more information on available command line options.

    Logging level is configured using the environmental variable ``ROBOTRACONTEUR_LOG_LEVEL``
    or the command line option ``--robotraconteur-log-level``. See Logging for more information.

    See Taps for more information on using taps.

    The node setup classes optionally initialize LocalTransport,
    TcpTransport, HardwareTransport, and/or IntraTransport. 
    Transports for more information.

    The LocalTransport.StartServerAsNodeName() or 
    LocalTransport.StartClientAsNodeName() are used to load the NodeID.
    See LocalTransport for more information on this procedure.

    :param node_name: (optional) The NodeName
    :type node_name: str
    :param tcp_port: (optional) The port to listen for incoming TCP clients
    :type tcp_port: int
    :param flags: (optional) The configuration flags
    :type flags: int
    :param allowed_overrides: (optional) The allowed override flags
    :type allowed_overrides: int
    :param node: (optional) The node to configure and manage lifecycle
    :type node: RobotRaconteur.RobotRaconteurNode
    :param argv: (optional) The command line argument vector. Default is ``sys.argv``
    """
    __slots__ = ["__setup", "__node", "tcp_transport", "local_transport", "hardware_transport", "intra_transport", "command_line_config" ]
    def __init__(self, node_name=None, tcp_port=None, flags=None, allowed_overrides=None, node=None, argv=None, config=None):
        if (config is not None):
            assert node_name is None and tcp_port is None and flags is None and allowed_overrides is None and argv is None
            self.__setup=RobotRaconteurPython.WrappedRobotRaconteurNodeSetup(config)
        else:
            if node_name is None:
                node_name = ""
            if tcp_port is None:
                tcp_port = 0
            if flags is None:
                flags = 0
            if node is None:
                node=RobotRaconteurPython.RobotRaconteurNode.s
            if allowed_overrides is None:
                allowed_overrides=0
            if argv is None:
                argv = []        
            self.__setup=RobotRaconteurPython.WrappedRobotRaconteurNodeSetup(node,node_name,tcp_port,flags, allowed_overrides, \
                RobotRaconteurPython.vectorstring(argv))
        self.browser_websocket_transport=self.__setup.GetBrowserWebSocketTransport()
        self.command_line_config = self.__setup.GetCommandLineConfig()
        self.__node=node
        
    def __enter__(self):
        return self
    
    def __exit__(self, etype, value, traceback):
        self.__node.Shutdown()

    def ReleaseNode(self):
        """
        Release the node from lifecycle management

        If called, RobotRaconteurNode.Shutdown() will not
        be called when the node setup instance is destroyed
        """
        if self.__setup is None:
            return
        self.__setup.ReleaseNode()
        
class ClientNodeSetup(RobotRaconteurNodeSetup):
    """
    Initializes a RobotRaconteurNode instance to default configuration for a client only node

    ClientNodeSetup is a subclass of RobotRaconteurNodeSetup providing default configuration for a
    RobotRaconteurNode instance that is used only to create outgoing client connections. 

    See CommandLineOptions for more information on available command line options.

    Note: String table and HardwareTransport are disabled by default. They can be enabled
    using command line options.

    By default, the configuration will do the following:

    1. Configure logging level from environmental variable or command line options. Defaults to `INFO` if
       not specified
    2. Configure tap if specified in command line options
    3. Register service types passed to service_types
    4. Start LocalTransport (default enabled)
       
       1. If `RobotRaconteurNodeSetupFlags_LOCAL_TRANSPORT_START_CLIENT` flag is specified, call
          LocalTransport.StartServerAsNodeName() with the specified node_name 
       2. Start LocalTransport discovery listening if specified in flags or on command line (default enabled)
       3. Disable Message Format Version 4 (default enabled) and/or String Table (default disabled) if 
          specified on command line 
    5. Start TcpTransport (default enabled)
    
       1. Disable Message Format Version 4 (default enabled) and/or String Table 
          (default disabled) if specified in flags or command line
       2. Start TcpTranport discovery listening (default enabled)
       3. Load TLS certificate and set if TLS is specified on command line (default disabled)
       4. Process WebSocket origin command line options
    6. Start HardwareTransport (default disabled)
       
       1. Disable Message Format Version 4 (default enabled) and/or String Table 
          (default disabled) if specified in flags or command line
    7. Start IntraTransport (default enabled)
       
       1. Disable Message Format Version 4 (default enabled) and/or String Table 
          (default disabled) if specified in flags or command line
    8. Disable timeouts if specified in flags or command line (default timeouts normal)

    Most users will not need to be concerned with these details, and can simply
    use the default configuration.

    :param node_name: (optional) The NodeName
    :type node_name: str
    :param node: (optional) The node to configure and manage lifecycle
    :type node: RobotRaconteur.RobotRaconteurNode
    :param argv: (optional) The command line argument vector. Default is ``sys.argv``
    """
    def __init__(self, node_name=None, node=None, argv=None):
        super(ClientNodeSetup,self).__init__(node_name,0, RobotRaconteurPython.RobotRaconteurNodeSetupFlags_CLIENT_DEFAULT, \
            RobotRaconteurPython.RobotRaconteurNodeSetupFlags_CLIENT_DEFAULT_ALLOWED_OVERRIDE,node,argv)
            

class UserLogRecordHandlerDirectorPython(RobotRaconteurPython.UserLogRecordHandlerDirector):
    def __init__(self, handler):
        super(UserLogRecordHandlerDirectorPython,self).__init__()
        self.handler = handler

    def HandleLogRecord(self,record):
        if self.handler is not None:
            self.handler(record)

class UserLogRecordHandler(RobotRaconteurPython.UserLogRecordHandlerBase):
    def __init__(self,handler):
        super(UserLogRecordHandler,self).__init__()
        director = UserLogRecordHandlerDirectorPython(handler)
        self._SetHandler(director,0)
        director.__disown__()

class TapFileReader(object):
    __slots__ = ["_fileobj"]
    def __init__(self, fileobj):
        self._fileobj = fileobj

    def ReadNextMessage(self):
        len_bytes = self._fileobj.read(8)
        if (len(len_bytes) < 8):
            return None
        message_len = RobotRaconteurPython.MessageLengthFromBytes(len_bytes)
        message_bytes = len_bytes + self._fileobj.read(message_len - 8)
        if (len(message_bytes) < message_len):
            return None
        m = RobotRaconteurPython.MessageFromBytes(message_bytes)
        return m

    def UnpackMessageElement(self, el, node=None):
        if node is None:
            node = RobotRaconteurPython.RobotRaconteurNode.s
        return UnpackMessageElement(el, "varvalue value", None, node)

def settrace():
    # Enable debugging in vscode if ptvsd has been loaded
    # This may potentially activate debugging when not expected if ptvsd has been imported
    # for some reason

    if 'ptvsd' in sys.modules:
        import ptvsd
        ptvsd.debug_this_thread()

# Based on https://github.com/akloster/aioweb-demo/blob/master/src/main.py

class WebFuture(object):
    
    def __init__(self):
        self.complete_handler=None
        self.exception_handler=None
        self.ret=None
        self.early_ret=None
        self.early_exp=None

    def handler(self, arg, exp):
        if (exp is not None):
            if (self.exception_handler is not None):
                self.exception_handler(exp)
            else:
                self.early_exp = exp
        else:
            self.ret=arg
            if (self.complete_handler is not None):
                self.complete_handler(arg)
            else:
                self.early_ret = arg

    def __await__(self):
        if self.early_ret is not None:
            return self.early_ret
        if self.early_exp is not None:
            raise self.early_exp
        yield self
        return self.ret

    __iter__ = __await__

class WebLoop:
    def __init__(self):
        self.coros = []
    def call_soon(self, coro):
        self.step(coro)
    def step(self, coro, arg=None):
        try:
            x = coro.send(arg)
            x.complete_handler = partial(self.step, coro)
            x.exception_handler = partial(self.fail,coro)
        except StopIteration:
            pass

    def fail(self, coro,arg=None):
        try:
            x = coro.throw(arg)
            x.complete_handler = partial(self.step, coro)
            x.exception_handler = partial(self.fail,coro)
        except StopIteration:
            pass

    @staticmethod
    def run(coro):
        loop=WebLoop()
        loop.call_soon(coro)

