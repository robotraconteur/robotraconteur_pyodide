from js import print_div
from RobotRaconteur.Client import *

print_div("Begin test_transport")

c1 = None

def i32_huge_cb(i32_huge, err):
    print_div ("i32_huge: " + str(i32_huge))
    print_div ("i32_huge error: " + str(err))

def d1_cb(d1, err):
    print_div ("d1: " + str(d1))
    print_div ("d1 error: " + str(err))
    c1.async_get_i32_huge(i32_huge_cb)

def connect_cb(c, err):
    global c1
    c1 = c
    print_div("connect error: " + str(err))
    c.async_get_d1(d1_cb)

RRN.AsyncConnectService("rr+ws://localhost:2222?service=RobotRaconteurTestService", None, None, None, connect_cb)


