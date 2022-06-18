from js import print_div
from RobotRaconteur.Client import *
import asyncio

print_div("Begin test_await")

c1 = None

async def test_await_func():

    c = await RRN.AsyncConnectService("rr+ws://localhost:22222?service=RobotRaconteurTestService", None, None, None, None)
    print_div("d1: " + str(await c.async_get_d1(None)))
    print_div("i32_huge: " + str(await c.async_get_i32_huge(None)))
    try:
        await c.async_set_d1(5,None)
    except GeneratorExit:
        raise
    except Exception as exp:
        print_div("Caught exception: " + str(exp))
    print_div("d1: " + str(await c.async_get_d1(None)))
    await c.async_set_d1(3.456,None)
    
    print_div("Done!")

#loop = RR.WebLoop()

asyncio.ensure_future(test_await_func())


