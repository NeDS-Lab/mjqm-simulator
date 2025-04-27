#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jan  7 16:21:22 2024

@author: dilettaolliaro
"""

from matplotlib.patches import Rectangle
from scipy.signal import savgol_filter
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib
import os.path
import csv
import re

###############################  SORTED DEMANDS ##########################################


def load_params(input_txt):
    
    # servers required by each clas starting from the smallest to the biggest one
    T = []
    # speeds for each job class starting from the smallest to the biggest one
    mus = []
    # probabilities for each job class starting from the smallest to the biggest one
    p = []

    nClasses = 0

    # Read the txt file to retrieve params
    with open(f'Inputs/{input_txt}.txt', 'r') as file:
        # Read and process each line
        for line in file:
            # Remove parentheses and split the line into values
            values = line.strip('()\n').split(', ')

            # Convert the values to the appropriate data types (int and float)
            dim = int(values[0])
            prob = float(values[1])
            speed = float(values[2])

            # Append the values to their respective lists
            T.append(dim)
            p.append(prob)
            mus.append(speed)
            nClasses += 1

    # Combine the lists into a list of tuples (if needed)
    data = list(zip(T, p, mus))

    # If you want to work with the sorted lists separately
    T, p, mus = zip(*data)
    T, p, mus = list(T), list(p), list(mus)
    
    p = np.array(p)
  
    return T, p, mus, nClasses

fsize = 150
legend_size = 200
label_size = 220
title_size = 195
tuplesize = (90, 70)
marker_size = 70**2
line_size = 25
tick_size = 180
l_pad = 40
asym_size = 20

cols = ['black', 'peru', 'darkorange', 'royalblue', 'crimson', 'purple', 'darkgreen',  'pink']
markers = ['P', 'o', 'v', 's',  'X', 'D', 'H', '<', '>']
styles = ['solid', 'dashdot', 'dotted', 'dashed', (0, (3, 5, 1, 5, 1, 5))]

cell = 'cellA'
n = 4096
nClasses = 29

'''cell = 'cellB'
n = 2048
nClasses = 26'''

inter_points = 10
# -1 : server filling
# 0: Most Server First
# 1: First-In First-Out
# i: SMASH w/ w=i
# Original Demands or Demands considered with the nearest higher power of two
# with respect to the original demand
type_str = ['Sorted_'+str(n)]#, 'powOfTwo_'+str(n)]

if cell == 'cellB':
    wins = [-3, -1, 0, 1, 2, 5, 10]
    types = [0 for i in range(len(wins))]
    index_asyms = [13, 52, 32, 27, 29, 34, 41]

    ys_bigResp = [7000, 5000, 3000, 500, 700, 1000, 2000]
    ys_resp = [2500, 750, 450, 15, 40, 100, 250]
    
    '''wins = [-3, -1, 0, 1, 5]
    types = [0 for i in range(len(wins))]
    index_asyms = [13, 52, 32, 27, 34]

    ys_bigResp = [700, 6000, 4700, 1000, 3000]
    ys_resp = [800, 550, 350, 7, 40]'''
    
    xs = [5.4, 5.4, 5.4, 5.4, 5.4, 5.4, 5.4]
    legend_locs = ['upper left', 'upper left', 'lower left', 'upper left', 'upper left', 'lower right']

    ylims_totResp = [5, 10000]
    ylims_totWait = [0.01, 10**5]
    ylims_bigResp = [100, 10000]
    ylims_smallResp = [1, 10000]
    ylims_smallWait = [0.01, 10000]
    
    ylims = [0.1, 10**5]
    ylims_v2 = [0.1, 10**5]
    
    xlims = [0.01, 10]
    st = styles[0]

elif cell == 'cellA':
    if n == 4096:
        #wins = [-1, 0, 1, 2, 5, 10, 50, 100]
        #types = [0 for i in range(len(wins))]
        #index_asyms = [39, 12, 31, 33, 37, 38, 13, 14]
        #ys_bigResp = [550, 400, 7, 15, 40, 100, 170, 200]
        #ys_resp = [70, 50, 4, 6, 8, 10, 25, 35]
        
        wins = [-3, -1, 0, 1, 2, 5, 10]
        types = [0 for i in range(len(wins))]
        index_asyms = [16, 39, 11, 30, 33, 35, 37]
        ys_bigResp = [550, 350, 400, 7, 15, 40, 100]
        ys_resp = [40, 30, 50, 2, 5, 8, 11]
        
        '''wins = [-3, -1, 0, 1, 5]
        types = [0 for i in range(len(wins))]
        index_asyms = [16, 39, 11, 30, 35]
        ys_bigResp = [650, 550, 400, 7, 40]
        ys_resp = [100, 70, 50, 4, 8]'''
        
        xs = [725 for w in wins]
        #xs = [602, 739, 561, 614, 667, 711]
        
        ylims_totResp = [1, 100]
        ylims_totWait = [10**-3, 100]
        ylims_bigResp = [1, 1000]
        ylims_smallResp = [1, 100]
        ylims_smallWait = [10**-3, 100]
        legend_locs = ['upper left', 'upper left', 'lower left', 'upper left', 'upper left', 'lower center']
        ylims = [1, 10**4]
        ylims_v2 = [0.001, 1000]
        
        xlims = [100, 10**3]
        st = styles[0]
    elif n == 3072:
        wins = [-1, 0, 1, 2, 5, 10]
        types = [0 for i in range(len(wins))]
        index_asyms = [41, 41, 19, 26, 36, 40]
        ys_bigResp = [800, 500, 20, 60, 120, 240]
        ys_resp = [10000, 6000, 200, 400, 800, 1400]
        
        xs = [370 for w in wins]
        #xs = [602, 739, 561, 614, 667, 711]
        
        ylims_totResp = [1, 100]
        ylims_totWait = [10**-3, 100]
        ylims_bigResp = [1, 1000]
        ylims_smallResp = [1, 100]
        ylims_smallWait = [10**-3, 100]
        
        ylims = [1, 10**4]
        ylims_v2 = [0.001, 1000]
        
        xlims = [100, 10**3]
        st = styles[0]
    elif n == 2048:
        wins = [0]
        types = [0 for i in range(len(wins))]
        
        
        index_asyms = [5]
        ys_bigResp = [800]
        ys_resp = [1000]
        
        xs = [50 for w in wins]
        #xs = [602, 739, 561, 614, 667, 711]
        
        ylims_totResp = [0.001, 100]
        ylims_totWait = [0.001, 100]
        ylims_bigResp = [0.001, 1000]
        ylims_smallResp = [0.001, 100]
        ylims_smallWait = [0.001, 100]
        
        ylims = [0.001, 10**4]
        ylims_v2 = [0.001, 1000]
        
        xlims = [1, 500]
        xlims_big = [1, 30]
        st = styles[0]
        

filenames = []
labels = []
typex = []

for w in range(len(wins)):
    typex.append(type_str[types[w]])
    s = f'Results/{cell}/OverLambdas-nClasses{nClasses}-N{n}-Win{wins[w]}-' + cell +'-' + typex[w] + '.csv'
    if os.path.isfile(s):
        filenames.append(s)
        if wins[w] == -3:
            l = 'BackFilling'
        elif wins[w] == -1:
            l = 'Server Filling'
        elif wins[w] == 0:
            l = 'Most Server First'    
        elif wins[w] == 1:
            l = 'First-In First-Out'
        else:
            l = f'SMASH w/ w = {wins[w]}' 
        labels.append(l)

Ts_sort, ps_sort, taus_sort, nClasses_sort = load_params(cell + '/' + cell + '_' + type_str[0])


lambdas = [[] for file in filenames]    
respTimes_tot = [[] for file in filenames]
respTimes_big = [[] for file in filenames]
respTimes_small = [[] for file in filenames]
waitTimes_tot = [[] for file in filenames]
waitTimes_big = [[] for file in filenames]
waitTimes_small = [[] for file in filenames]
wasted_servers = [[] for file in filenames]

utils = [[] for file in filenames]
actual_util = []

for f in range(len(filenames)):  
  input_txt = cell + '/' + cell + '_' + typex[f]
  Ts, ps, taus, nClasses = load_params(input_txt)
  with open(filenames[f]) as csv_file:
        if f == 0:
            df = pd.read_csv(filenames[f], delimiter = ',')
        else:    
            df = pd.read_csv(filenames[f], delimiter = ';')

        for index, row in df.iterrows():
               lambdas[f].append(float(row['Arrival Rate']))
               respTimes_tot[f].append(float(row['RespTime Total']))
               respTimes_big[f].append(float(row[f'T{max(Ts)} RespTime']))
               respTimes_small[f].append(float(row[f'T{min(Ts)} RespTime']))
               waitTimes_big[f].append(float(row[f'T{max(Ts)} Waiting']))
               waitTimes_small[f].append(float(row[f'T{min(Ts)} Waiting']))
               waitTimes_tot[f].append(float(row['WaitTime Total']))
               wasted_servers[f].append(float(row['Wasted Servers']))
             
asymptotes = [lambdas[i][index_asyms[i]] for i in range(len(index_asyms))]
lims = []

############################### TOTAL RESPONSE TIME ##########################################

plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = fsize
matplotlib.rcParams['xtick.major.pad'] = 8
matplotlib.rcParams['ytick.major.pad'] = 8
fix, ax = plt.subplots(figsize=tuplesize)        

for f in range(0, len(filenames)):
    input_txt = cell + '/' + cell + '_' + typex[f]
    Ts, ps, taus, nClasses = load_params(input_txt)
    summ_util = 0
    for t in range(len(Ts)):
        if Ts_sort[t] <= Ts[t]:
            summ_util += asymptotes[f]*ps[t]*Ts_sort[t]*taus[t]*(1/n)
        else:
            summ_util += asymptotes[f]*ps[t]*Ts[t]*taus[t]*(1/n)
    actual_util.append(summ_util)
    lims.append(lambdas[f].index(asymptotes[f]))
   
for f in range(len(filenames)):       
    
    x_data = lambdas[f][:lims[f]+1]
    y_data = respTimes_tot[f][:lims[f]+1]
    y_interp = savgol_filter(y_data, 3, 2)
   
    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=labels[f], ls=st, lw=line_size)
    
    #ax.plot(lambdas[f][:lims[f]+1], respTimes_tot[f][:lims[f]+1], color = cols[f], label = labels[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size)
    #ax.plot(lambdas[f][lims[f]:], respTimes_tot[f][lims[f]:], color = cols[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size, mfc='k')
    util = round(actual_util[f]*100, 1)
    if (cell == 'cellA' and wins[f]!=0) or cell == 'cellB': 
        plt.text(x = xs[f], y = ys_resp[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
        plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
    
ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Response Time $\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Overall Response Time vs. Arrival Rate", fontsize=title_size)
plt.xscale('log')
plt.yscale('log')
plt.ylim(ylims_totResp[0], ylims_totResp[1])
plt.xlim(xlims[0], xlims[1])
#plt.yticks(fontsize=tick_size)
#plt.xticks(fontsize=tick_size, which = 'minor')
ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)

ax.legend(fontsize = legend_size, loc = legend_locs[0])

ax.grid()
plt.savefig('lambdasVsTotRespTime-' + cell + f'_{n}.pdf', bbox_inches='tight')

############################### SMALL CLASS RESPONSE TIME ##########################################

plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = fsize
fix, ax = plt.subplots(figsize=tuplesize)        

for f in range(len(filenames)):        
    
    x_data = lambdas[f][:lims[f]+1]
    y_data = respTimes_small[f][:lims[f]+1]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=labels[f], ls=st, lw=line_size)
           
    #ax.plot(lambdas[f][:lims[f]+1], respTimes_small[f][:lims[f]+1], color = cols[f], label = labels[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size)
    #ax.plot(lambdas[f][lims[f]:], respTimes_small[f][lims[f]:], color = cols[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size, mfc='k')
    util = round(actual_util[f]*100, 1)
    if (cell == 'cellA' and wins[f]!=0) or cell == 'cellB': 
        plt.text(x = xs[f], y = ys_resp[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
        plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
    
ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Response Time $\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Response Time for the Smallest Class vs. Arrival Rate", fontsize=title_size)
plt.xscale('log')
plt.yscale('log')
plt.ylim(ylims_smallResp[0], ylims_smallResp[1])
plt.xlim(xlims[0], xlims[1])
#plt.yticks(fontsize=tick_size)
#plt.xticks(fontsize=tick_size)
ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
#ax.legend(fontsize = legend_size, loc = legend_locs[1])
ax.grid()
plt.savefig('lambdasVsSmallRespTime-' + cell + f'_{n}.pdf', bbox_inches='tight')


############################### BIG CLASS RESPONSE TIME ##########################################


plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = fsize
fix, ax = plt.subplots(figsize=tuplesize)        

for f in range(len(filenames)):        
     
    x_data = lambdas[f][:lims[f]+1]
    y_data = respTimes_big[f][:lims[f]+1]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=labels[f], ls=st, lw=line_size)
      
    #ax.plot(lambdas[f][:lims[f]+1], respTimes_big[f][:lims[f]+1], color = cols[f], label = labels[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size)
    #ax.plot(lambdas[f][lims[f]:], respTimes_big[f][lims[f]:], color = cols[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size, mfc='k')
    util = round(actual_util[f]*100, 1)
    if (cell == 'cellA' and wins[f]!=0) or cell == 'cellB': 
        plt.text(x = xs[f], y = ys_bigResp[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
        plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
        
ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Response Time $\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Response Time for the Biggest Class vs. Arrival Rate", fontsize=title_size)
plt.xscale('log')
plt.yscale('log')
plt.ylim(ylims_bigResp[0], ylims_bigResp[1])
plt.xlim(xlims[0], xlims[1])
ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
#plt.yticks(fontsize=tick_size)
#plt.xticks(fontsize=tick_size)
#ax.legend(fontsize = legend_size, loc = legend_locs[2], ncols=2)


ax.grid()
plt.savefig('lambdasVsBigRespTime-' + cell + f'_{n}.pdf', bbox_inches='tight')


############################### TOTAL WAITING TIME ##########################################

plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = fsize
fix, ax = plt.subplots(figsize=tuplesize)        

for f in range(len(filenames)):        
     
    x_data = lambdas[f][:lims[f]+1]
    y_data = waitTimes_tot[f][:lims[f]+1]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=labels[f], ls=st, lw=line_size)
      
    #ax.plot(lambdas[f][:lims[f]+1], waitTimes_tot[f][:lims[f]+1], color = cols[f], label = labels[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size)
    #ax.plot(lambdas[f][lims[f]:], waitTimes_tot[f][lims[f]:], color = cols[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size, mfc='k')
    if (cell == 'cellA' and wins[f]!=0) or cell == 'cellB':    
        plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
    
ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Waiting Time $\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Overall Waiting Time vs. Arrival Rate", fontsize=title_size)
plt.xscale('log')
plt.yscale('log')
plt.ylim(ylims_totWait[0], ylims_totWait[1])
plt.xlim(xlims[0], xlims[1])
ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
#plt.yticks(fontsize=tick_size)
#plt.xticks(fontsize=tick_size)
#ax.legend(fontsize = legend_size, loc = legend_locs[3])


ax.grid()
plt.savefig('lambdasVstotWaitTime-' + cell + f'_{n}.pdf', bbox_inches='tight')


############################### SMALL CLASS WAITING TIME ##########################################

plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = fsize
fix, ax = plt.subplots(figsize=tuplesize)        

for f in range(len(filenames)):    

    x_data = lambdas[f][:lims[f]+1]
    y_data = waitTimes_small[f][:lims[f]+1]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=labels[f], ls=st, lw=line_size)
          
           
    #ax.plot(lambdas[f][:lims[f]+1], waitTimes_small[f][:lims[f]+1], color = cols[f], label = labels[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size)
    #ax.plot(lambdas[f][lims[f]:], waitTimes_small[f][lims[f]:], color = cols[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size, mfc='k')

    #util = round(actual_util[f]*100, 1)
    #plt.text(x = xs[f], y = ys[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
    if (cell == 'cellA' and wins[f]!=0) or cell == 'cellB':     
        plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
    
ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Waiting Time $\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Waiting Time for the Smallest Class vs. Arrival Rate", fontsize=title_size)
plt.xscale('log')
plt.yscale('log')
plt.ylim(ylims_smallWait[0], ylims_smallWait[1])
plt.xlim(xlims[0], xlims[1])
#plt.yticks(fontsize=tick_size)
#plt.xticks(fontsize=tick_size)
ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
#ax.legend(fontsize = legend_size, loc = legend_locs[4])


ax.grid()
plt.savefig('lambdasVsSmallWaitTime-' + cell + f'_{n}.pdf', bbox_inches='tight')


############################### BIG CLASS WAITING TIME ##########################################


plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = fsize
fix, ax = plt.subplots(figsize=tuplesize)        

for f in range(len(filenames)):        
     

    x_data = lambdas[f][:lims[f]+1]
    y_data = waitTimes_big[f][:lims[f]+1]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=labels[f], ls=st, lw=line_size)
      
    #ax.plot(lambdas[f][:lims[f]+1], waitTimes_big[f][:lims[f]+1], color = cols[f], label = labels[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size)
    #ax.plot(lambdas[f][lims[f]:], waitTimes_big[f][lims[f]:], color = cols[f], ls = st, marker = markers[f], lw = line_size, markersize = marker_size, mfc='k')

    #util = round(actual_util[f]*100, 1)
    #plt.text(x = xs[f], y = ys[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
    if (cell == 'cellA' and wins[f]!=0) or cell == 'cellB': 
        plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw=asym_size)
    
ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Waiting Time $\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Waiting Time for the Biggest Class vs. Arrival Rate", fontsize=title_size)
plt.xscale('log')
plt.yscale('log')
plt.ylim(ylims_v2[0], ylims_v2[1])
plt.xlim(xlims[0], xlims[1])
ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
#plt.yticks(fontsize=tick_size)
#plt.xticks(fontsize=tick_size)
#ax.legend(fontsize = legend_size, loc = legend_locs[5])


ax.grid()
plt.savefig('lambdasVsBigWaitTime-' + cell + f'_{n}.pdf', bbox_inches='tight')

######################################################################################
