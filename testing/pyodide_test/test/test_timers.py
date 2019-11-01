from js import print_div
import RobotRaconteur as RR
RRN = RR.RobotRaconteurNode.s


print_div("Begin test_timers")

i=0

def timer_cb(ev):
    
    if ev.stopped:
        return
    print_div("Got timer callback!")
    global i
    i+=1
    if i > 10:            
        timer.Stop()
        print_div("Stopped")
    
        

timer = RRN.CreateTimer(1,timer_cb,False)
timer.Start()

def post_cb():
    print_div("Post cb!")

RRN.PostToThreadPool(post_cb)


