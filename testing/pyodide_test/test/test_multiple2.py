from js import print_div
from RobotRaconteur.Client import *

print_div("Begin test_multiple2")

c1 = None

def d1_cb(d1, err):
    print_div ("d1: " + str(d1))
    print_div ("d1 error: " + str(err))
    c1.async_get_i32_huge(i32_huge_cb)

def connect_cb(c, err):
    global c1
    c1 = c
    print_div("connect error: " + str(err))
    c.async_get_d1(d1_cb)

RRN.SetLogLevel(RR.LogLevel_Debug)

RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, connect_cb)
RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, connect_cb)
RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, connect_cb)
RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, connect_cb)



