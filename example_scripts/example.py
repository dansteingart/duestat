#! python 
##Author: Dan Steingart
##Date Started: 2021-10-04
##Notes: Simple Example Script (start, not complete)

import requests as r
from time import time, sleep
from numpy import *
base  = "http://localhost:3200/setting/"
durl  = "http://localhost:3200/data/"
exp_s = "http://localhost:3200/exp_start/"
exp_e = "http://localhost:3200/exp_stop/"

#helper functions
def set_p(kp):
    #only KP for now
    data = {}
    data['setpid'] = True
    data['kp'] = kp
    return r.post(base,data=data).json()

def manual(pval,gval=None):
    data = {}
    data['mode'] = "manual"
    data['dac0'] = pval
    if gval != None: data['dac1'] = gval
    r.post(base,data)
    return r.post(base,data=data).json()

def sine(amp,rate):
    data = {}
    data['mode'] = "sine"
    data['amp'] = amp
    data['rate'] = rate
    return r.post(base,data=data).json()

def set_r(res):
    data = {'res1':res}
    return r.post(base,data=data).json()

def pstat(target,gval=None):
    data = {}
    data['mode'] = "pstat"
    data['target'] = target
    if gval != None: data['dac1'] = gval
    return r.post(base,data=data).json()

def gstat(target,gval=None):
    data = {}
    data['mode'] = "gstat"
    data['target'] = target
    if gval != None: data['dac1'] = gval

    return r.post(base,data=data).json()

def ocv(): 
    data = {}
    data['mode'] = "ocv"
    return r.post(base,data=data).json()

def get_last(): return r.get(durl).json()


def start_exp(exp): 
    return r.post(exp_s,data=exp).json()

def stop_exp(): 
    return r.post(exp_e).json()

def sleepget(per):
    sleep(.1)
    for i in range(per):
        print(get_last())
        sleep(1)

#show off a bit

set_r(22000) #assuming a Rf of 22K
set_p(.1) #control proportionality factor

manual(1.5,gval=1)
sleepget(2)

pstat(1.5,gval=0)
sleepget(2)

manual(1.5,gval=1)
sleepget(5)

ocv()
sleepget(5)

gstat(.005*1e-3,gval=1)
sleepget(5)

pstat(-.5,gval=1)
sleepget(5)

gstat(0,gval=1)
sleepget(5)

gstat(.01*1e-3,gval=1)
sleepget(5)

gstat(-.01*1e-3,gval=1)
sleepget(5)

gstat(.02*1e-3,gval=1)
sleepget(5)

ocv()
sleepget(5)
