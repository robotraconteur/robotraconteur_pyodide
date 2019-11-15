from js import print_div
from RobotRaconteur.Client import *

import time

print_div("Begin test_await_performance")

c1 = None

async def test_await_func():

    c = await RRN.AsyncConnectService("rr+ws://localhost:2222?service=RobotRaconteurTestService", None, None, None, None)
    
    t = time.time()
    for _ in range(1000):        
        f = await c.async_get_struct1(None)
        #print_div("f = " + str(f.dat1))
        #f = RRN.NewStructure("com.robotraconteur.testing.TestService1.teststruct1",c)
        #print_div("d1 = "  + str(f.dat1))
    elapsed = time.time() - t

    print_div("Time for 1000 await calls: %f" % elapsed)
    
    print_div("Done!")

loop = RR.WebLoop()
loop.call_soon(test_await_func())


