#! python 
##Author: Dan Steingart
##Date Started: 2021-10-04
##Notes: CV Example Script (start, not complete)

import requests as r
from time import time, sleep
import duestat 
from numpy import *

due = duestat.duestat("http://localhost:3200",22000)
experiment = {}
experiment['name'] = "Dan_Test_02"

due.start_exp(experiment)
due.ocv()
sleep(5)

for j in range(8):
    for i in linspace(-.5,.5,100):
        due.pstat(i,gval=1.5)
        sleep(.01)
    
    for i in linspace(.5,-.5,100):
        due.pstat(i,gval=1.5)
        sleep(.01)
due.ocv()
sleep(5)
due.stop_exp()
