#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Feb 23 18:00:06 2024

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

fsize = 70
legend_size = 70
label_size = 80
title_size = 80
tuplesize = (35, 30)
marker_size = 70**2
line_size = 12
tick_size = 70
l_pad = 40
asym_size = 8

#sample_methods = ["BoundedPareto", "Deterministic", "Exponential", "Pareto", "Uniform"]
sample_methods = ["BoundedPareto", "Deterministic", "Exponential", "Uniform"]
#folders = ['bPar', 'Det', 'Exp', 'Par', 'Uni']
folders = ['bPar', 'Det', 'Exp', 'Uni']
legend_locs = ['upper left', 'lower left', 'lower left', 'upper left', 'upper left', 'lower right']
#cols = ['peru', 'darkorange', 'royalblue', 'crimson', 'purple', 'darkgreen', 'darkcyan', 'pink']
cols = ['#b76e79', '#fc8d2b', 'darkcyan', '#903C11', '#f74d89']
styles = ['solid', 'dotted', 'dashed', 'dashdot', (0, (3, 5, 1, 5, 1, 5))]
markers = ['o', 'v', 's',  'X', 'D', 'H', 'P', '<', '>']

#cell = 'cellA'
for cell in ['cellA','cellB']:
    if cell == 'cellA':
        n = 3072
    else:
        n = 2048
    
    
    st = styles[0]
    combos = [[32, 1024], [16, 512], [8, 256], [4, 256]]
    
    types = [f'Sorted_{n}']
    
    
    for ty in range(len(types)):
        
        if cell == 'cellA':
            
            nClasses = 29
            # Inner lists represent bPar Det Exp Uni Par
            #matrix_asyms = [[54, 88, 53, 82, 54]]
            matrix_asyms = [[19,19,19,19]]
            index_asyms = matrix_asyms[ty]
            
            ys_totResp = [10**2, 5000, 10, 10**3]
            
            ys_totWait = [10**-1, 10**3, 10**-3, 10**1]
            
            xs = [400 for w in index_asyms]
            
            ylims_totResp = [1, 10**4]
           
            ylims_totWait = [10**-7, 10**4]
             
            xlims = [0.5, 10**3]    
    
        elif cell == 'cellB':
            
            nClasses = 26
            #Sorted [32, 1024], [16, 512], [8, 256], [4, 256]
            # Inner lists represent bPar Det Exp Uni Par
            #matrix_asyms = [[28, 31, 28, 30, 29]]
            matrix_asyms = [[19,19,19,19]]
            #matrix_asyms = [[28, 44, 51, 55, 57], [28, 44, 51, 55, 57], [28, 44, 51, 55, 57]]
            index_asyms = matrix_asyms[ty]
            
            
            ys_totResp = [10**2, 10**5, 10**3, 10**4]
            
            ys_totWait = [10**-1, 10**5, 10**1, 10**3]
            
            xs = [3.5 for i in range(len(index_asyms))]
    
            ylims_totResp = [5, 10**6]        
            
            ylims_totWait = [10**-2, 10**6]
            
            
            xlims = [10**-2, 10]
        
        
        ###################### GET FILES IN INPUT, SET LABELS #####################
        
        filenames = []
        labels = []
        txts = []
       
        for s in range(len(sample_methods)):
            fnm = 'Results/' + cell + f'/{folders[s]}/OverLambdas-nClasses{nClasses}-N{n}-Win1-{sample_methods[s]}-'+cell+f'-{types[ty]}.csv'
            txt = cell + '/' + cell + f'_{types[ty]}'
            l = f'{sample_methods[s]}'
        
            filenames.append(fnm)
            labels.append(l)
            txts.append(txt)
        
        ######################## DATA STRUCTURES FOR DATA #########################
    
        lambdas = [[] for file in filenames]    
        
        actual_util = []
        asymptotes = []
        lims = []
    
        respTimes_tot = [[] for file in filenames]
        
        waitTimes_tot = [[] for file in filenames]
        
        probs = [[] for file in filenames]
        cores = [[] for file in filenames]
        times = [[] for file in filenames]
        nClass = [[] for file in filenames]
        
        ########################### GET INPUT PARAMETERS ##########################
        
        for f in range(len(filenames)):
            cores[f], probs[f], times[f], nClass[f] = load_params(txts[f])
            
    
        ############################## RETRIEVE DATA ##############################
        
        for f in range(0, len(filenames)):
            
            if cell == 'cellA' and ('Deterministic' in filenames[f] or 'Uniform' in filenames[f]):
                df = pd.read_csv(filenames[f], delimiter = ',')
            else:
                df = pd.read_csv(filenames[f], delimiter = ';')
                
            df = pd.read_csv(filenames[f], delimiter = ';')
            
            for index, row in df.iterrows():
                 lambdas[f].append(float(row['Arrival Rate']))
                 
                 respTimes_tot[f].append(float(row['RespTime Total'])) 
                 waitTimes_tot[f].append(float(row['WaitTime Total']))
                 
       
        ########################### COMPUTE UTILISATION ###########################
        
        for f in range(0, len(filenames)):
            asymptotes.append(lambdas[f][index_asyms[f]])
        
        for f in range(0, len(filenames)):
            summ_util = 0
            for t in range(len(cores[0])):
                if cores[0][t] <= cores[f][t]:
                    summ_util += asymptotes[f]*probs[0][t]*cores[0][t]*times[f][t]*(1/n)
                else:
                    summ_util += asymptotes[f]*probs[f][t]*cores[f][t]*times[f][t]*(1/n)
            actual_util.append(summ_util)
            lims.append(lambdas[f].index(asymptotes[f]))    
    
        ############################ TOTAL RESPONSE TIME ##########################
        
        plt.figure(dpi=1200)
        plt.rc('font',**{'family':'serif','serif':['Palatino']})
        plt.rc('text', usetex=True)
        matplotlib.rcParams['font.size'] = fsize
        matplotlib.rcParams['xtick.major.pad'] = 8
        matplotlib.rcParams['ytick.major.pad'] = 8
        fix, ax = plt.subplots(figsize=tuplesize)        
        
        for f in range(len(filenames)):       
            
            x_data = lambdas[f][:lims[f]+1]
            y_data = respTimes_tot[f][:lims[f]+1]
           
            #ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
            ax.plot(x_data, y_data, color=cols[f], label=labels[f], ls=styles[f], lw=line_size)
            
            util = round(actual_util[f]*100, 1)
            
            plt.text(x = xs[f], y = ys_totResp[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
            plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
            
        ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
        ax.set_ylabel("Avg. Response Time $\quad[$s$]$", fontsize=label_size)
        ax.set_title(f"Avg. Overall Response Time vs. Arrival Rate", fontsize=title_size)
        plt.xscale('log')
        plt.yscale('log')
        plt.ylim(ylims_totResp[0], ylims_totResp[1])
        plt.xlim(xlims[0], xlims[1])
        ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
        ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
        
        ax.legend(fontsize = legend_size, loc = legend_locs[0])
        
        ax.grid()
        plt.savefig('Figures/resize-lambdasVsTotRespTimeOverDDP-' + cell + f'_{n}.pdf')
        
        ############################ TOTAL WAITING TIME ##########################
        
        plt.figure(dpi=1200)
        plt.rc('font',**{'family':'serif','serif':['Palatino']})
        plt.rc('text', usetex=True)
        matplotlib.rcParams['font.size'] = fsize
        matplotlib.rcParams['xtick.major.pad'] = 8
        matplotlib.rcParams['ytick.major.pad'] = 8
        fix, ax = plt.subplots(figsize=tuplesize)        
        
        for f in range(len(filenames)):       
            
            x_data = lambdas[f][:lims[f]+1]
            y_data = waitTimes_tot[f][:lims[f]+1]
            y_interp = savgol_filter(y_data, 3, 2)
           
            #ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
            ax.plot(x_data, y_data, color=cols[f], label=labels[f], ls=styles[f], lw=line_size)
            
            util = round(actual_util[f]*100, 1)
            
            plt.text(x = xs[f], y = ys_totWait[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
            plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
            
        ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
        ax.set_ylabel("Avg. Waiting Time $\quad[$s$]$", fontsize=label_size)
        ax.set_title(f"Avg. Overall Waiting Time vs. Arrival Rate", fontsize=title_size)
        plt.xscale('log')
        plt.yscale('log')
        plt.ylim(ylims_totWait[0], ylims_totWait[1])
        plt.xlim(xlims[0], xlims[1])
        ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
        ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
        
        ax.legend(fontsize = legend_size, loc = legend_locs[0])
        
        ax.grid()
        plt.savefig('Figures/resize-lambdasVsTotWaitTimeOverDDP-' + cell + f'_{n}.pdf')

