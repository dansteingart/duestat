#! python 
##Author: Dan Steingart
##Date Started: 2021-10-08
##Notes: See pithy3 library here or just sub matplotlib

from pithy3 import *
import requests as req
import duestat

due = duestat.duestat("http://localhost:3200",22000)
exps = due.get_exps()
df = due.get_exp_data(exps[-1])
    
ts = (df['TIME (us)'] - df['TIME (us)'].iloc[0])/1000
vol = df['CELL (V)'] - df['DAC1 (V)']
cur = ((df['DAC0 (V)'] - df['CELL (V)'])/df['RES (ohm)'])*1000

subplot(2,1,1)
plot(ts,vol)
subplot(2,1,2)
plot(ts,cur)
showme()
clf()
