# Copyright 2011-2019 Wason Technology, LLC
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

def NewStructure(StructType,obj=None,node=None):
    if (hasattr(obj,'rrinnerstub')):
        obj=obj.rrinnerstub
    return RobotRaconteurPython._NewStructure(StructType, obj, node)

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
   
    outerstub.rrinnerstub=stub;
    outerstub.rrlock=threading.RLock()
    return outerstub

class AsyncRequestDirectorImpl(RobotRaconteurPython.AsyncRequestDirector):
    def __init__(self,handler,isvoid,Type,stub,node):
        super(AsyncRequestDirectorImpl,self).__init__()
        self._handler=handler
        self._isvoid=isvoid
        self._Type=Type
        self._node=node
        self._stub=stub

    def handler(self,m,error_code,errorname,errormessage):

        if (self._isvoid):
            if (error_code!=0):
                err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
                self._handler(err)
                return
            else:
                self._handler(None)
        else:
            if (error_code!=0):
                err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
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
    m=RobotRaconteurPython.vectorptr_messageelement()
    i=0
    for p in type1.Parameters:
        a=PackMessageElement(args[i],p,stub)
        m.append(a)
        i+=1
    handler=args[i]
    if (len(args) > i+1):
        timeout=args[i+1]
    else:
        timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE
    return async_call(stub.async_FunctionCall,(name,m,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(False,type1.ReturnType,stub,stub.RRGetNode()))

def stub_async_functioncallvoid(stub,name,type1,*args):
    m=RobotRaconteurPython.vectorptr_messageelement()
    i=0
    for p in type1.Parameters:
        a=PackMessageElement(args[i],p,stub)
        m.append(a)
        i+=1
    handler=args[i]
    if (len(args) > i+1):
        timeout=args[i+1]
    else:
        timeout=-1

    return async_call(stub.async_FunctionCall,(name,m,adjust_timeout(timeout)),AsyncRequestDirectorImpl,handler,directorargs=(True,type1.ReturnType,stub,stub.RRGetNode()))

def stub_async_functioncallgenerator(stub,name,type1,*args):
    m=RobotRaconteurPython.vectorptr_messageelement()
    i=0
    param_type = None
    for p in type1.Parameters:
        if (p.ContainerType != RobotRaconteurPython.DataTypes_ContainerTypes_generator):
            a=PackMessageElement(args[i],p,stub)
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
    pass

class CallbackClient(object):
    def __init__(self):
        self.Function=None

class PipeEndpoint(object):
    def __init__(self,innerpipe, type, obj=None):
        self.__innerpipe=innerpipe
        self.__type=type
        self.PipeEndpointClosedCallback=None
        self._PacketReceivedEvent=EventHook()
        self._PacketAckReceivedEvent=EventHook()
        self.__obj=obj

    @property
    def Index(self):
        return self.__innerpipe.GetIndex()

    @property
    def Endpoint(self):
        return self.__innerpipe.GetEndpoint()

    @property
    def Available(self):
        return self.__innerpipe.Available()

    @property
    def IsUnreliable(self):
        return self.__innerpipe.IsUnreliable()

    @property
    def Direction(self):
        return self.__innerpipe.Direction()

    @property
    def RequestPacketAck(self):
        return self.__innerpipe.GetRequestPacketAck()

    @RequestPacketAck.setter
    def RequestPacketAck(self,value):
        self.__innerpipe.SetRequestPacketAck(value)

    @property
    def IgnoreReceived(self):
        return self.__innerpipe.GetIgnoreReceived()

    @IgnoreReceived.setter
    def IgnoreReceived(self,value):
        self.__innerpipe.SetIgnoreReceived(value)

    def AsyncClose(self,handler,timeout=2):
        return async_call(self.__innerpipe.AsyncClose,(adjust_timeout(timeout),),AsyncVoidReturnDirectorImpl,handler)

    def AsyncSendPacket(self, packet, handler):
        m=PackMessageElement(packet,self.__type,self.__obj,self.__innerpipe.GetNode())
        return async_call(self.__innerpipe.AsyncSendPacket,(m,),AsyncUInt32ReturnDirectorImpl,handler)

    def ReceivePacket(self):
        m=self.__innerpipe.ReceivePacket()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerpipe.GetNode())

    def PeekNextPacket(self):
        m=self.__innerpipe.PeekNextPacket()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerpipe.GetNode())

    def TryReceivePacketWait(self, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE, peek=False):
        #TODO: Add timeout back
        m=RobotRaconteurPython.MessageElement()
        r=self.__innerpipe.TryReceivePacket(m, peek)
        return (r, UnpackMessageElement(m,self.__type,self.__obj,self.__innerpipe.GetNode()))
    
    @property
    def PacketReceivedEvent(self):
        return self._PacketReceivedEvent
    
    @PacketReceivedEvent.setter
    def PacketReceivedEvent(self, evt):
        if (evt is not self._PacketReceivedEvent):
            raise RuntimeError("Invalid operation")
    
    @property
    def PacketAckReceivedEvent(self):
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

    def handler(self, innerendpoint, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
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
    def __init__(self,innerpipe,obj=None):
        self._innerpipe=innerpipe        
        self._obj=obj

    def AsyncConnect(self,*args):
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
        return self._innerpipe.GetMemberName()        
    
    @property
    def Direction(self):
        return self._innerpipe.Direction()
    

class WireConnection(object):
    def __init__(self,innerwire, type, obj=None):
        self.__innerwire=innerwire
        self.__type=type
        self.WireConnectionClosedCallback=None
        self._WireValueChanged=EventHook()
        self.__obj=obj

    @property
    def Endpoint(self):
        return self.__innerwire.GetEndpoint()

    @property
    def Direction(self):
        return self.__innerwire.Direction()

    def AsyncClose(self,handler,timeout=2):
        return async_call(self.__innerwire.AsyncClose,(adjust_timeout(timeout),),AsyncVoidReturnDirectorImpl,handler)

    @property
    def InValue(self):
        m=self.__innerwire.GetInValue()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerwire.GetNode())

    @property
    def OutValue(self):
        m=self.__innerwire.GetOutValue()
        return UnpackMessageElement(m,self.__type,self.__obj,self.__innerwire.GetNode())

    @OutValue.setter
    def OutValue(self,value):
        m=PackMessageElement(value,self.__type,self.__obj,self.__innerwire.GetNode())
        return self.__innerwire.SetOutValue(m)

    @property
    def LastValueReceivedTime(self):
        return self.__innerwire.GetLastValueReceivedTime()

    @property
    def LastValueSentTime(self):
        return self.__innerwire.GetLastValueSentTime()

    @property
    def InValueValid(self):
        return self.__innerwire.GetInValueValid()

    @property
    def OutValueValid(self):
        return self.__innerwire.GetOutValueValid()
        
    @property
    def IgnoreInValue(self):
        return self.__innerwire.GetIgnoreInValue()

    def TryGetInValue(self):
        ts=RobotRaconteurPython.TimeSpec()
        m=RobotRaconteurPython.MessageElement()
        res=self.__innerwire.TryGetInValue(m,ts)
        if not res:
            return (False,None, None)
        return (True, UnpackMessageElement(m,self.__type,self.__obj,self.__innerwire.GetNode()), ts)

    def TryGetOutValue(self):
        ts=RobotRaconteurPython.TimeSpec()
        m=RobotRaconteurPython.MessageElement()
        res=self.__innerwire.TryGetOutValue(m,ts)
        if not res:
            return (False,None, None)
        return (True, UnpackMessageElement(m,self.__type,self.__obj,self.__innerwire.GetNode()), ts)

    @IgnoreInValue.setter
    def IgnoreInValue(self,value):
        self.__innerwire.SetIgnoreInValue(value)
 
    @property
    def WireValueChanged(self):
        return self._WireValueChanged
    
    @WireValueChanged.setter
    def WireValueChanged(self,evt):
        if (evt is not self._WireValueChanged):
            raise RuntimeError("Invalid operation")

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

    def handler(self, innerendpoint, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
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

    def handler(self,m,ts,error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
            self._handler((None, None), err)
            return
        value=UnpackMessageElement(m,self.__innerpipe.Type,self.__obj,self.__innerpipe.GetNode())
        self._handler((value, ts), None)

       
class Wire(object):
    def __init__(self,innerpipe,obj=None):
        self._innerpipe=innerpipe        
        self._obj=obj

    def Connect(self):
        innerendpoint=self._innerpipe.Connect()
        outerendpoint=WireConnection(innerendpoint,self._innerpipe.Type,self._obj)
        director=WireConnectionDirector(outerendpoint,self._innerpipe.Type,self._obj,innerendpoint)
        innerendpoint.SetRRDirector(director,0)
        director.__disown__()
        return outerendpoint

    def AsyncConnect(self,handler,timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):

        return async_call(self._innerpipe.AsyncConnect,(adjust_timeout(timeout),),WireAsyncConnectHandlerImpl,handler,directorargs=(self._innerpipe,self._obj))

    @property
    def MemberName(self):
        return self._innerpipe.GetMemberName()

    @property
    def Direction(self):
        return self._innerpipe.Direction()

    def AsyncPeekInValue(self, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        return async_call(self._innerpipe.AsyncPeekInValue, (adjust_timeout(timeout),), WireAsyncPeekReturnDirectorImpl, handler, directorargs=(self._innerpipe,self._obj))

    def AsyncPeekOutValue(self, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
        return async_call(self._innerpipe.AsyncPeekOutValue, (adjust_timeout(timeout),), WireAsyncPeekReturnDirectorImpl, handler, directorargs=(self._innerpipe,self._obj))

    def AsyncPokeOutValue(self, value, handler, timeout=RobotRaconteurPython.RR_TIMEOUT_INFINITE):
         m=PackMessageElement(value,self._innerpipe.Type,self._obj,self._innerpipe.GetNode())
         return async_call(self._innerpipe.AsyncPokeOutValue, (m,adjust_timeout(timeout)), AsyncVoidReturnDirectorImpl, handler)

    
class ServiceInfo2(object):
    def __init__(self,info):
        self.Name=info.Name
        self.RootObjectType=info.RootObjectType
        self.RootObjectImplements=list(info.RootObjectImplements)
        self.ConnectionURL=list(info.ConnectionURL)
        self.NodeID=RobotRaconteurPython.NodeID(str(info.NodeID))
        self.NodeName=info.NodeName
        self.Attributes=UnpackMessageElement(info.Attributes,"varvalue{string} value")

class NodeInfo2(object):
    def __init__(self,info):
        self.NodeID=RobotRaconteurPython.NodeID(str(info.NodeID))
        self.NodeName=info.NodeName
        self.ConnectionURL=list(info.ConnectionURL)

class WrappedClientServiceListenerDirector(RobotRaconteurPython.ClientServiceListenerDirector):
    def __init__(self,callback):
        self.callback=callback

        super(WrappedClientServiceListenerDirector,self).__init__()

    def Callback(self,code):

       self.callback(self.stub,code,None)


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
                return const.Name, [int(i) for i in s3.split(',')]
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
    for e in servicedef.Objects:
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
                
                
    for n,v in elem_o.values():
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

    def handler(self, innerstub2, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
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

    def handler(self, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
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

    def handler(self, istr, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
            self._handler(None,err)
            return       
        self._handler(istr, None)

class AsyncUInt32ReturnDirectorImpl(RobotRaconteurPython.AsyncUInt32ReturnDirector):
    def __init__(self,handler):
        super(AsyncUInt32ReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, e, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
            self._handler(None,err)
            return       
        self._handler(e, None)

class AsyncTimerEventReturnDirectorImpl(RobotRaconteurPython.AsyncTimerEventReturnDirector):
    def __init__(self,handler):
        super(AsyncTimerEventReturnDirectorImpl,self).__init__()
        self._handler=handler

    def handler(self, ev, error_code, errorname, errormessage):        
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

    def handler(self, error_code,errorname,errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
            self._handler(err)
            return

class GeneratorClient(object):
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
        
    def handler(self, gen, error_code, errorname, errormessage):
        if (error_code!=0):
            err=RobotRaconteurPythonError.RobotRaconteurExceptionUtil.ErrorCodeToException(error_code,errorname,errormessage)
            self._handler(None,err)
            return
        
        gen2=GeneratorClient(gen, self._return_type, self._param_type, self._obj, self._node)
        self._handler(gen2, None)

       
_trace_hook=sys.gettrace()

class ServiceSubscriptionClientID(object):
    def __init__(self, *args):
        if (len(args) == 1):
            self.NodeID=args[0].NodeID
            self.ServiceName=args[0].ServiceName
        elif (len(args) == 2):
            self.NodeID=args[0];
            self.ServiceName=args[1];
        else:
            self.NodeID=None
            self.ServiceName=None

    def __eq__(self, other):
        if (not hasattr(other,'NodeID') or not hasattr(other,'ServiceName')):
            return False;
        return (self.NodeID == other.NodeID) and (self.ServiceName == other.ServiceName)

    def __neq__(self,other):
        return not self == other

    def __hash__(self):
      return hash((str(self.NodeID), self.ServiceName))

class ServiceSubscriptionFilterNode(object):
    def __init__(self):
        self.NodeID=None
        self.NodeName=None
        self.Username=None
        self.Credentials=None

class ServiceSubscriptionFilter(object):
    def __init__(self):
        self.Nodes=[]
        self.ServiceNames=[]
        self.TransportSchemes=[]
        self.Predicate=None
        self.MaxConnections=1000000

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
    def __init__(self, subscription):
        self._subscription=subscription        
        self._ServiceDetected=EventHook()
        self._ServiceLost=EventHook()
        director=WrappedServiceInfo2SubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
    
    
    def GetDetectedServiceInfo2(self):
        o=dict()
        c1=self._subscription.GetDetectedServiceInfo2()
        for c2 in c1.items():
            id1=ServiceSubscriptionClientID(c2[0])
            stub=ServiceInfo2(c2[1])
            o[id1]=stub        
        return o

    def Close(self):
        self._subscription.Close()
            
    @property
    def ServiceDetected(self):
        return self._ServiceDetected
    
    @ServiceDetected.setter
    def ServiceDetected(self, evt):
        if (evt is not self._ServiceDetected):
            raise RuntimeError("Invalid operation")
    
    @property
    def ServiceLost(self):
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
        except: pass

    def ClientDisconnected(self, subscription, id, client):
        s = self._subscription()
        if (s is None):
            return

        client2=s._GetClientStub(client)

        try:
            s.ClientDisconnected.fire(s, ServiceSubscriptionClientID(id), client2)
        except: pass

class ServiceSubscription(object):
    def __init__(self, subscription):
        self._subscription=subscription        
        self._ClientConnected=EventHook()
        self._ClientDisconnected=EventHook()
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
        o=dict()
        c1=self._subscription.GetConnectedClients()
        for c2 in c1.items():
            id1=ServiceSubscriptionClientID(c2[0])
            stub=self._GetClientStub(c2[1])
            o[id1]=stub        
        return o

    def Close(self):
        self._subscription.Close()

    @property
    def ConnectRetryDelay(self):
        return self._subscription.GetConnectRetryDelay() / 1000.0

    @ConnectRetryDelay.setter
    def ConnectRetryDelay(self,value):
        if (value < 1):
            raise Exception("Invalid ConnectRetryDelay value")
        self._subscription.SetConnectRetryDelay(int(value*1000.0))

    def SubscribeWire(self, wire_name):
        s=self._subscription.SubscribeWire(wire_name)
        return WireSubscription(s)

    def SubscribePipe(self, pipe_name):
        s=self._subscription.SubscribePipe(pipe_name)
        return PipeSubscription(s)
    
    @property
    def ClientConnected(self):
        return self._ClientConnected
    
    @ClientConnected.setter
    def ClientConnected(self, evt):
        if (evt is not self._ClientConnected):
            raise RuntimeError("Invalid operation")
    
    @property
    def ClientDisconnected(self):
        return self._ClientDisconnected
    
    @ClientDisconnected.setter
    def ClientDisconnected(self, evt):
        if (evt is not self._ClientDisconnected):
            raise RuntimeError("Invalid operation")
            
class WrappedWireSubscriptionDirectorPython(RobotRaconteurPython.WrappedWireSubscriptionDirector):
    def __init__(self,subscription):
        super(WrappedWireSubscriptionDirectorPython,self).__init__()
        self._subscription=weakref.ref(subscription)

    def WireValueChanged(self, subscription, value, time):
        s=self._subscription()
        if (s is None):
            return

        try:            
            v=RobotRaconteurPython._UnpackMessageElement(value, None, None, None, True)            
            s.WireValueChanged.fire(s,v,time)
        except:
            traceback.print_exc()

class WireSubscription(object):
    def __init__(self, subscription):
        self._subscription=subscription
        director=WrappedWireSubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
        self._WireValueChanged=EventHook()

    def _UnpackValue(self, m):
        return RobotRaconteurPython._UnpackMessageElement(m, None, None, None, True)

    @property
    def InValue(self):
        return self._UnpackValue(self._subscription.GetInValue())

    @property
    def InValueWithTimeSpec(self):
        t=RobotRaconteurPython.TimeSpec()
        m=self._subscription.GetInValue(t)
        return (self._UnpackValue(m), t)

    @property
    def ActiveWireConnectionCount(self):
        return self._subscription.GetActiveWireConnectionCount()

    @property
    def IgnoreInValue(self):
        return self._subscription.GetIgnoreInValue()

    @IgnoreInValue.setter
    def IgnoreInValue(self, ignore):
        self._subscription.SetIgnoreInValue(ignore)

    def SetOutValueAll(self, value):        
        iter=RobotRaconteurPython.WrappedWireSubscription_send_iterator(self._subscription)
        try:
            while iter.Next() is not None:
                m=PackMessageElement(value,iter.GetType(),iter.GetStub())
                iter.SetOutValue(m)                
        finally:
            del iter

    def Close(self):
        self._subscription.Close()
        
    @property
    def WireValueChanged(self):
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
    def __init__(self, subscription):
        self._subscription=subscription
        director=WrappedPipeSubscriptionDirectorPython(self)
        subscription.SetRRDirector(director,0)
        director.__disown__()
        self._PipePacketReceived=EventHook()

    def _UnpackValue(self, m):
        return RobotRaconteurPython._UnpackMessageElement(m, None, None, None, True)

    def ReceivePacket(self):
        return self._UnpackValue(self._subscription.ReceivePacket())

    def TryReceivePacket(self):
        return self.TryReceivePacketWait(0)

    def TryReceivePacketWait(self, timeout=-1):
        m=RobotRaconteurPython.MessageElement()
        res=self._subscription.TryReceivePacket(m)
        if (not res):
            return (False, None)
        else:
            return (True, self._UnpackValue(m))

    @property
    def Available(self):
        return self._subscription.Available()

    def AsyncSendPacketAll(self, packet):        
        iter=RobotRaconteurPython.WrappedPipeSubscription_send_iterator(self._subscription)
        try:
            while iter.Next() is not None:
                m=PackMessageElement(packet,iter.GetType(),iter.GetStub())
                iter.AsyncSendPacket(m)                
        finally:
            del iter

    @property
    def ActivePipeEndpointCount(self):
        return self._subscription.GetActivePipeEndpointCount()

    def Close(self):
        return self._subscription.Close()
    
    @property
    def PipePacketReceived(self):
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

def SubscribeService(node, service_types, filter_=None):

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

    
    sub1=RobotRaconteurPython.WrappedSubscribeService(node, service_types2, filter2)
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

class RobotRaconteurNodeSetup(object):
    def __init__(self, node_name, tcp_port, flags, node=None):
        if node_name is None:
            node_name = ""
        if tcp_port is None:
            tcp_port = 0
        if flags is None:
            flags = 0
        if node is None:
            node=RobotRaconteurPython.RobotRaconteurNode.s
        self.__setup=RobotRaconteurPython.WrappedRobotRaconteurNodeSetup(node,node_name,tcp_port,flags)    
        # TODO: Add transport
        # self.tcp_transport=self.__setup.GetTcpTransport()
        
        self.__node=node
        
    def __enter__(self):
        return self
    
    def __exit__(self, etype, value, traceback):
        self.__node.Shutdown()
        
class ClientNodeSetup(RobotRaconteurNodeSetup):
    def __init__(self, node_name=None, flags=RobotRaconteurPython.RobotRaconteurNodeSetupFlags_CLIENT_DEFAULT, node=None):
        super(ClientNodeSetup,self).__init__(node_name,0,flags,node)

def settrace():
    #This function enables debugging for the threads started by the ThreadPool
    #You may see a warning in Eclipse; it can safely be ignored.
    t=_trace_hook
    if (t is not None):
        sys.settrace(t)

# Based on https://github.com/akloster/aioweb-demo/blob/master/src/main.py

class WebFuture(object):
    
    def __init__(self):
        self.complete_handler=None
        self.exception_handler=None
        self.ret=None

    def handler(self, arg, exp):
        if (exp is not None):
            if (self.exception_handler is not None):
                self.exception_handler(exp)
        else:
            self.ret=arg
            if (self.complete_handler is not None):
                self.complete_handler(arg)

    def __await__(self):
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

