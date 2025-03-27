#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Feb 27 10:02:02 2024

@author: dilettaolliaro
"""

from matplotlib.patches import Rectangle
from scipy.signal import savgol_filter
import matplotlib.pyplot as plt
import matplotlib as mpl
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
title_size = 200
tuplesize = (100, 80)
marker_size = 70**2
line_size = 25
tick_size = 180
l_pad = 40
asym_size = 20

#sample_methods = ["BoundedPareto", "Deterministic", "Exponential", "Pareto", "Uniform"]
sample_methods = ["Exponential"]#, "Deterministic", "Exponential", "Uniform", "Pareto"]
#folders = ['bPar', 'Det', 'Exp', 'Par', 'Uni']
folders = ['Exp']#, 'Det', 'Exp', 'Uni', 'Par']




#cell = 'cellB'

for cell in ['cellA','cellB']:
    for s in range(len(sample_methods)):
        
        if cell == 'cellA':
            
            n = 3072
            nClasses = 29
            lim = 53
            
            ylims_wait = [10**-2, 10**4]
            
            xlims = [0.5, 300]
            
            leg_loc = 'upper left'
    
        elif cell == 'cellB':
            
            n = 2048
            nClasses = 26
            lim = 28
            
            ylims_wait = [10**-1, 10**4]
            
            xlims = [10**-2, 5]
            
            leg_loc = 'lower right'
        
        colors = mpl.colormaps.get_cmap('viridis').resampled(nClasses).colors
        
        ###################### GET FILES IN INPUT, SET LABELS #####################
        
    
        filename = 'Results/' + cell + f'/{folders[s]}/OverLambdas-nClasses{nClasses}-N{n}-Win1-{sample_methods[s]}-'+cell+f'-Sorted_{n}.csv'
        txt = cell + '/' + cell + f'_Sorted_{n}'
        
        lambdas = []    
    
        waitTimes_allClasses = [[] for c in range(nClasses)]
        
        cores, probs, times, nClass = load_params(txt)
        
        labels = []
        
        for c in range(nClasses):
           # labels.append(r'$\mathcal{T}_{c} = $' + f'{cores[c]}')
           labels.append(r'$\mathcal{T}_{' + f'{c+1}' + r'} = $' + f'{cores[c]}')
        ############################## RETRIEVE DATA ##############################
            
        df = pd.read_csv(filename, delimiter = ';')
        
        for index, row in df.iterrows():
             lambdas.append(float(row['Arrival Rate']))
             
             for c in range(nClass):
                 waitTimes_allClasses[c].append(float(row[f'T{cores[c]} Waiting']))
    
        
       
    
        ##################### PLOT WAITING TIME OF ALL CLASSES ####################
        
        plt.figure(dpi=1200)
        plt.rc('font',**{'family':'serif','serif':['Palatino']})
        plt.rc('text', usetex=True)
        matplotlib.rcParams['font.size'] = fsize
        matplotlib.rcParams['xtick.major.pad'] = 8
        matplotlib.rcParams['ytick.major.pad'] = 8
        fix, ax = plt.subplots(figsize=tuplesize)        
        
        for c in range(nClasses):       
            
            x_data = lambdas[:lim]
            y_data = waitTimes_allClasses[c][:lim]
           
            #ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
            ax.plot(x_data, y_data, color=colors[c], label=labels[c], ls='solid', lw=line_size)
                  
        ax.set_xlabel("Arrival Rate $\quad[$s$^{-1}]$", fontsize=label_size)
        ax.set_ylabel("Avg. Waiting Time $\quad[$s$]$", fontsize=label_size)
        ax.set_title("Avg. Waiting Time per Class vs. Arrival Rate", fontsize=title_size)
    
        plt.xscale('log')
        plt.yscale('log')
        plt.ylim(ylims_wait[0], ylims_wait[1])
        plt.xlim(xlims[0], xlims[1])
        ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
        ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
        #ax.legend(fontsize = legend_size, loc='center left', bbox_to_anchor=(1, 0.5), ncol = 2)
        ax.legend(fontsize = legend_size, loc = leg_loc, ncol = 3)
        ax.grid()
        plt.tight_layout()
        plt.savefig('Figures/lambdasVsWaitTime-AllClasses-' + sample_methods[s] + '-' + cell + f'_{n}.pdf')
    
   