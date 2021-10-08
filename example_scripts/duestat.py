#! python 
##Author: Dan Steingart
##Date Started: 2021-10-04
##Notes: duestat library attempt

import requests as r
from time import time, sleep

class duestat:
    def __init__(self,loc,res,kp=0.1,gval = 0):
        self.loc = loc
        self.res = res
        self.kp = kp
        self.setting_url = loc+"/setting/"
        self.data_url = loc+"/data/"
        self.exp_start_url = loc+"/exp_start/"
        self.exp_stop_url = loc+"/exp_stop/"
        self.exp_list_url = loc+"/get_experiment_list/"
        self.exp_csv = loc+"/get_experiment_data/"
        self.exp_meta = loc+"/get_experiment_data/"

        self.set_p(self.kp)
        self.set_res(self.res)
        self.gval = gval

    def set_res(self,res):
        self.res = res
        data = {'res1':res}
        return r.post(self.setting_url,data=data).json()

    #helper functions
    def set_p(self,kp):
        #only KP for now
        self.kp = kp
        data = {}
        data['setpid'] = True
        data['kp'] = kp
        return r.post(self.setting_url,data=data).json()

    def manual(self,pval,gval=None):
        data = {}
        data['mode'] = "manual"
        data['dac0'] = pval
        if gval != None: data['dac1'] = gval
        r.post(base,data)
        return r.post(self.setting_url,data=data).json()
    
    def sine(self,amp,rate):
        data = {}
        data['mode'] = "sine"
        data['amp'] = amp
        data['rate'] = rate
        return r.post(self.setting_url,data=data).json()
    
    
    def pstat(self,target,gval=None):
        self.gval = gval
        data = {}
        data['mode'] = "pstat"
        data['target'] = target
        if gval != None: 
            self.gval = gval
            data['dac1'] = gval
        return r.post(self.setting_url,data=data).json()
    
    def gstat(self,target,gval=None):
        data = {}
        data['mode'] = "gstat"
        data['target'] = target
        if gval != None: 
            self.gval = gval
            data['dac1'] = gval
        return r.post(self.setting_url,data=data).json()
    
    def ocv(self): 
        data = {}
        data['mode'] = "ocv"
        return r.post(self.setting_url,data=data).json()
    
    def get_last(self): return r.get(self.data_url).json()
    
    def start_exp(self,exp): 
        return r.post(self.exp_start_url,data=exp).json()
    
    def stop_exp(self): 
        return r.post(self.exp_stop_url).json()
        
    def get_exps(self): 
        return r.get(self.exp_list_url).json()

    def get_exp_data(self,exp): 
        import pandas as pd
        return(pd.read_csv(self.exp_csv+exp))

    def get_exp_meta(self,exp): 
        return r.get(self.exp_meta+exp).json()
        

#test it
if __name__ == "__main__":
    due = duestat("http://localhost:3200",22000)
    exps = due.get_exps()
    df = due.get_exp_data(exps[-3])
    
    print(df)
