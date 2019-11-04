from js import print_div
from RobotRaconteur.Client import *

from matplotlib import pyplot as plt
import matplotlib
print_div("matplotlib backend: " + str(matplotlib.get_backend()))
plt.figure()
plt.plot([1,2,3])
plt.show()
print_div("Done?")
plt.clf()


async def test_plot():
    a=0
    while True:        
        a+=0.1
        plt.clf()
        plt.plot([a,2,3])
        await RRN.AsyncSleep(0.1,None)        

RR.WebLoop.run(test_plot())
