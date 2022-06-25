from js import print_div
#from RobotRaconteur.Client import *
import asyncio

import time
import hashlib
import random

print_div("Begin sha256")

N = 100000

t = time.time()

for i in range(N):
    hashlib.sha256(str(random.random()).encode('utf-8')).hexdigest()
elapsed = time.time() - t

print_div(f"Time for {N} sha256 calls: {elapsed}")

print_div("Done!")



