from js import print_div
from RobotRaconteur.Client import *
import traceback
import asyncio

print_div("Begin test_subscriber")

RRN.SetLogLevelFromString("DEBUG")
RR.SetPythonTracebackPrintExc(True)

async def test_subscriber_func():
    
    sub = RRN.SubscribeService("rr+ws://localhost:2222?service=RobotRaconteurTestService")
    #await RRN.AsyncSleep(0.5, None)
    while True:
        try:
            c = await sub.AsyncGetDefaultClient(None)
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
        except:
            traceback.print_exc()
   
    print_div("Done!")

loop = asyncio.get_event_loop()

async def run_program():
    try:
        await test_subscriber_func()
    except:
        traceback.print_exc()

loop.create_task(run_program())



