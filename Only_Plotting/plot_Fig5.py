#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jul 29 16:12:23 2024

@author: dilettaolliaro
"""

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Mar  2 14:13:35 2024

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

fsize = 150
legend_size = 150
label_size = 220
title_size = 180
tuplesize = (85, 75)
marker_size = 70**2
line_size = 25
tick_size = 180
l_pad = 40
asym_size = 20

sample_methods = ["Exponential"]
folders = ['Exp']
#cols = ['peru', 'darkorange', 'royalblue', 'crimson', 'purple', 'darkgreen', 'darkcyan', 'pink']
cols = ['peru', 'royalblue', 'crimson', 'purple', 'darkgreen', '#FF00FF', '#00FFFF']
styles = ['solid', 'dotted', 'dashed', 'dashdot', (0, (3, 5, 1, 5, 1, 5)), (0, (5, 10)), (0, (1, 1))]
markers = ['o', 'v', 's',  'X', 'D', 'H', 'P', '<', '>']

#cell = 'cellA'

st = styles[0]
combos = [[32, 1024], [16, 512], [8, 256], [4, 256]]

for cell in ['cellA', 'cellB']:
    for s in range(len(sample_methods)):
        
        
        if cell == 'cellA':
                
            n = 3072
            nClasses = 29
            #BoundedPareto Deterministic Exponential Uniform Pareto
            #Cols Order: Sorted, 32-1024, 16-512, 8-256, 4-256
            #matrix_asyms = [[53, 22, 32, 51, 95]]
            #matrix_asyms = [[53, 22, 33, 51, 95, 46, 49]]
            matrix_asyms = [[-1, -1, -1, -1, -1, -1, -1]]    
            index_asyms = matrix_asyms[s]
            
            coordinates = [100, 1.2, 10, 30, 300, 700, 3.5]
                       
            ys_totResp = coordinates#[100, 3.5, 10, 30, 300, 500, 700]
            ys_bigResp =  coordinates#[100, 3.5, 10, 30, 300, 500, 700]
            ys_smallResp =  coordinates#[100, 3.5, 10, 30, 300, 500, 700]
            
            ys_totWait = [10**-4, 10**-2, 10, 10**2, 10**3]
            xs = [600 for w in index_asyms]
            
            ylims_totResp = [10**-1, 10**4]#10**3]
            ylims_bigResp = [10**-1, 10**4]#10**3]
            ylims_smallResp = [10**-1, 10**4]#10**3]
            
            ylims_totWait = [10**-8, 10**5]
             
            xlims = [0.5, 1700]    
            
            legend_locs = ['upper left', 'upper left', 'upper left', 'upper left']
    
        elif cell == 'cellB':
            
            n = 2048
            nClasses = 26
            #BoundedPareto Deterministic Exponential Uniform Pareto
            #Cols Order: Sorted, 32-1024, 16-512, 8-256, 4-256
            #matrix_asyms = [[28, 44, 51, 55, 57, 47, 21]]
            #matrix_asyms = [[28, 44, 51, 55, 57], [28, 44, 51, 55, 57], [28, 44, 51, 55, 57]]
            matrix_asyms = [[-1, -1, -1, -1, -1, -1, -1]]
            index_asyms = matrix_asyms[s]
            
            coordinates = [2.5, 10, 200, 1000, 5000, 50, 0.5]
            ys_totResp = coordinates
            ys_bigResp = coordinates
            ys_smallResp = coordinates
            
            ys_totWait = [10, 100, 1000, 4000, 7000, 10**4]
            
            xs = [10 for i in range(len(index_asyms))]
    
            ylims_totResp = [10**-1, 10**5]
            ylims_bigResp = [10**-1, 10**5]
            ylims_smallResp = [10**-1, 10**5]
            
            
            ylims_totWait = [10**-2, 15000]
            
            
            xlims = [10**-2, 25]
            
            legend_locs = ['upper left', 'lower left', 'upper left', 'upper left']
    
        
        
        ###################### GET FILES IN INPUT, SET LABELS #####################
        
        filenames = []
        labels = []
        txts = []
    
        filenames.append('Results/' + cell + f'/{folders[s]}/OverLambdas-nClasses{nClasses}-N{n}-Win1-{sample_methods[s]}-'+cell+f'-Sorted_{n}.csv')
        txts.append(cell + '/' + cell + f'_Sorted_{n}')
        labels.append('Original demands')
        for combo in combos:
            fnm = 'Results/' + cell + f'/{folders[s]}/OverLambdas-nClasses{nClasses}-N{n}-Win1-{sample_methods[s]}-'+cell+f'-hyper-Ts{combo[0]}-Tb{combo[1]}-N{n}.csv'
            txt = cell + '/' + cell + f'_hyper-Ts{combo[0]}-Tb{combo[1]}-N{n}'
            l = r'$\mathcal{T}_s =$' + fr' {combo[0]}, ' + r'$\mathcal{T}_b = $' +f'{combo[1]}'
        
            filenames.append(fnm)
            labels.append(l)
            txts.append(txt)
        filenames.append('Results/' + cell + f'/{folders[s]}/OverLambdas-nClasses{nClasses}-N{n}-Win1-{sample_methods[s]}-'+cell+f'-hyper-LowerPowOfTwo-N{n}.csv')
        filenames.append('Results/' + cell + f'/{folders[s]}/OverLambdas-nClasses{nClasses}-N{n}-Win1-{sample_methods[s]}-'+cell+f'-hyper-UpperPowOfTwo-N{n}.csv')
    
        txts.append(cell + '/' + cell + f'_hyper-LowerPowOfTwo-N{n}')
        txts.append(cell + '/' + cell + f'_hyper-UpperPowOfTwo-N{n}')
    
        labels.append('Lower Pow. of 2')
        labels.append('Upper Pow. of 2')
        ######################## DATA STRUCTURES FOR DATA #########################
    
        lambdas = [[] for file in filenames]    
        
        actual_util = []
        asymptotes = []
        lims = []
    
        respTimes_tot = [[] for file in filenames]
        respTimes_small = [[] for file in filenames]
        respTimes_big = [[] for file in filenames]
        
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
            
            if cell == 'cellA' and (sample_methods[s] in ['Uniform', 'Deterministic']):
                df = pd.read_csv(filenames[f], delimiter = ',')
            else:
                df = pd.read_csv(filenames[f], delimiter = ';')
    
            
            for index, row in df.iterrows():
                 lambdas[f].append(float(row['Arrival Rate']))
                 respTimes_tot[f].append(float(row['RespTime Total'])) 
                 
                 respTimes_big[f].append(float(row[f'T{max(cores[f])} RespTime']))  
                 respTimes_small[f].append(float(row[f'T{min(cores[f])} RespTime']))
        
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
            
            plt.text(x = xs[f], y = ys_totResp[f], s = f'{util}\\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
            plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
            
        ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
        ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
        ax.set_title(f"Avg. Overall Response Time vs. Arrival Rate", fontsize=title_size)
        plt.xscale('log')
        plt.yscale('log')
        plt.ylim(ylims_totResp[0], ylims_totResp[1])
        plt.xlim(xlims[0], xlims[1])
        ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
        ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
        
        ax.legend(fontsize = legend_size, loc = legend_locs[0])
        
        ax.grid()
        plt.savefig('Figures/lambdasVsTotRespTime-' + sample_methods[s] + '-' + cell + f'_{n}.pdf')
        
        ########################## BIG CLASS RESPONSE TIME ########################
        
        plt.figure(dpi=1200)
        plt.rc('font',**{'family':'serif','serif':['Palatino']})
        plt.rc('text', usetex=True)
        matplotlib.rcParams['font.size'] = fsize
        matplotlib.rcParams['xtick.major.pad'] = 8
        matplotlib.rcParams['ytick.major.pad'] = 8
        
        fix, ax = plt.subplots(figsize=tuplesize)        
        
        for f in range(len(filenames)):       
            
            x_data = lambdas[f][:lims[f]+1]
            y_data = respTimes_big[f][:lims[f]+1]
            y_interp = savgol_filter(y_data, 3, 2)
           
            #ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
            ax.plot(x_data, y_data, color=cols[f], label=labels[f], ls=styles[f], lw=line_size)
            
            util = round(actual_util[f]*100, 1)
            
            plt.text(x = xs[f], y = ys_bigResp[f], s = f'{util}\\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
            plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
            
        ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
        ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
        ax.set_title(f"Avg. Response Time of Biggest Class vs. Arrival Rate", fontsize=title_size)
        plt.xscale('log')
        plt.yscale('log')
        plt.ylim(ylims_bigResp[0], ylims_bigResp[1])
        plt.xlim(xlims[0], xlims[1])
        ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
        ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
        
        ax.legend(fontsize = legend_size, loc = legend_locs[1])
        
        ax.grid()
        plt.savefig('Figures/lambdasVsBiggestRespTime-' + sample_methods[s] + '-' + cell + f'_{n}.pdf')
    
        ######################### SMALL CLASS RESPONSE TIME #######################
        
        plt.figure(dpi=1200)
        plt.rc('font',**{'family':'serif','serif':['Palatino']})
        plt.rc('text', usetex=True)
        matplotlib.rcParams['font.size'] = fsize
        matplotlib.rcParams['xtick.major.pad'] = 8
        matplotlib.rcParams['ytick.major.pad'] = 8
        fix, ax = plt.subplots(figsize=tuplesize)        
        
        for f in range(len(filenames)):       
            
            x_data = lambdas[f][:lims[f]+1]
            y_data = respTimes_small[f][:lims[f]+1]
            y_interp = savgol_filter(y_data, 3, 2)
           
            #ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
            ax.plot(x_data, y_data, color=cols[f], label=labels[f], ls=styles[f], lw=line_size)
            
            util = round(actual_util[f]*100, 1)
            
            plt.text(x = xs[f], y = ys_smallResp[f], s = f'{util}\\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
            plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dotted', lw = asym_size)
            
        ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
        ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
        ax.set_title(f"Avg. Response Time of Smallest Class vs. Arrival Rate", fontsize=title_size)
        plt.xscale('log')
        plt.yscale('log')
        plt.ylim(ylims_smallResp[0], ylims_smallResp[1])
        plt.xlim(xlims[0], xlims[1])
        ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
        ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
        
        ax.legend(fontsize = legend_size, loc = legend_locs[2])
        
        ax.grid()
        plt.savefig('Figures/lambdasVsSmallestRespTime-' + sample_methods[s] + '-' + cell + f'_{n}.pdf')
    
  