from js import print_div
from RobotRaconteur.Client import *

print_div("Begin test_robot")

c1 = None

async def test_await_func():

    try:
        c = await RRN.AsyncConnectService("rr+ws://192.168.1.133:58653?service=robot", None, None, None, None)
        print_div("d1: " + str((await c.robot_state.AsyncPeekInValue(None))[0].joint_position))
    except Exception as exp:
        print_div("Caught exception: " + str(exp))
    print_div("Done!")


loop = RR.WebLoop()
loop.call_soon(test_await_func())


