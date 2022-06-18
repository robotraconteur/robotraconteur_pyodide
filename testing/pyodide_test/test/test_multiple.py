from js import print_div
from RobotRaconteur.Client import *
import traceback
import asyncio

print_div("Begin test_multiple")

c1 = None

RRN.SetLogLevel(RR.LogLevel_Debug)

async def test_await_func():
    try:
        c1_task = await RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, None)
        c2_task = await RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, None)
        c3_task = await RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, None)
        c4_task = await RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, None)

        c1 = c1_task
        c2 = c2_task
        c3 = c3_task
        c4 = c4_task

        c1_d1_task = c1.async_get_d1(None)
        c2_d1_task = c2.async_get_d1(None)
        c3_d1_task = c3.async_get_d1(None)
        c4_d1_task = c4.async_get_d1(None)

        print_div("c1 d1: " + str(await c1_d1_task))
        print_div("c2 d1: " + str(await c2_d1_task))
        print_div("c3 d1: " + str(await c3_d1_task))
        print_div("c4 d1: " + str(await c4_d1_task))
    except:
        traceback.print_exc()
    


asyncio.ensure_future(test_await_func())


