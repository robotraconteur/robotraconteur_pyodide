from js import print_div
from RobotRaconteur.Client import *
import asyncio

RRN.SetLogLevel(RR.LogLevel_Debug)

print_div("Begin test_wire")

c1 = None

async def test_wire_func():

    c = await RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, None)
    w = await c.broadcastwire.AsyncConnect(None)
    await RRN.AsyncSleep(1,None)
    for _ in range(50):
        print_div(w.InValue)
        await RRN.AsyncSleep(0.01,None)

    
    print_div("Done!")

asyncio.ensure_future(test_wire_func())


