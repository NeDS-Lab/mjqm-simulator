#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Jul  9 19:48:20 2024

@author: dilettaolliaro
"""

from matplotlib.patches import Rectangle
import matplotlib.pyplot as plt
from matplotlib import rc
import pandas as pd
import matplotlib

fsize = 150
legend_size = 160
label_size = 220
title_size = 195
tuplesize = (90, 70)
marker_size = 100
line_size = 20
tick_size = 180
l_pad = 40
asym_size = 20 

T = 100
N = 200
mu_s = 1
ps = 0.5
pb = 0.5

filename = 'Results/Validation/gspnRes.csv'
iftaus = 1

mubs_gspn = []    
wastedHOL_gspn = []    

with open(filename) as csv_file:

    df = pd.read_csv(filename, delimiter = ',')

    for index, row in df.iterrows():
         
         mubs_gspn.append(float(row['Big Class Rate']))
         wastedHOL_gspn.append(float(row['Wasted Servers']))

#loads = 0.85
cols = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728','#9467bd', '#8c564b', 
          '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']


filename = 'Results/Validation/simRes.csv'

mubs_sim = []    
wastedHOL_sim = []

    
with open(filename) as csv_file:
    
    df = pd.read_csv(filename, delimiter = ',')

    for index, row in df.iterrows():
         
         mubs_sim.append(float(row['Big Class Rate']))
         wastedHOL_sim.append(float(row['Wasted Servers']))
 
if iftaus:         
    taus_gspn = [1/mubs_gspn[i] for i in range(len(mubs_gspn))]        
    taus_sim = [1/mubs_sim[i] for i in range(len(mubs_sim))]        
    
    plt.figure(dpi=1200)
    plt.rc('font',**{'family':'serif','serif':['Palatino']})
    plt.rc('text', usetex=True)
    matplotlib.rcParams['font.size'] = fsize
    matplotlib.rcParams['xtick.major.pad'] = 8
    matplotlib.rcParams['ytick.major.pad'] = 8
    fix, ax = plt.subplots(figsize=tuplesize) 
        
    ax.plot(taus_gspn, wastedHOL_gspn, color='black', label = 'Exact Results', ls='solid', lw = 35)         
    ax.plot(taus_sim, wastedHOL_sim, color='blue', ls = ' ', marker='o', label = 'Simulation Results', markersize=120, fillstyle = 'none', markeredgewidth=20)
        
    ax.set_xlabel(r"$\tau_b\quad [s]$", fontsize=label_size)
    ax.set_ylabel(r"Avg. Wasted Servers", fontsize=label_size)
    ax.set_title("Avg. Wasted Servers vs. Big Class Service Time", fontsize=title_size)
    plt.xscale('log')
    plt.yscale('linear')
    plt.xlim(0.002, 10**2)
    
    ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
    ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
    ax.legend(fontsize = legend_size, loc = 'upper right')
    ax.grid()
    plt.savefig('Figures/validation.pdf')

else:
    plt.figure(dpi=1200)
    plt.rc('font',**{'family':'serif','serif':['Palatino']})
    plt.rc('text', usetex=True)
    matplotlib.rcParams['font.size'] = fsize
    matplotlib.rcParams['xtick.major.pad'] = 8
    matplotlib.rcParams['ytick.major.pad'] = 8
    fix, ax = plt.subplots(figsize=tuplesize) 

    ax.plot(mubs_gspn, wastedHOL_gspn, color='black', label = 'Exact Results', ls='solid', lw = 35)         
    ax.plot(mubs_sim, wastedHOL_sim, color='blue', ls = ' ', marker='o', label = 'Simulation Results', markersize=120, fillstyle = 'none', markeredgewidth=20)


    ax.set_xlabel(r"$\mu_b\quad [s^{-1}]$", fontsize=label_size)
    ax.set_ylabel(r"Avg. Wasted Servers", fontsize=label_size)
    ax.set_title("Avg. Wasted Servers vs. Big Class Service Rate", fontsize=title_size)
    plt.xscale('log')
    plt.yscale('linear')
    plt.xlim(0.01, 500)

    ax.tick_params(axis='both', which='major', labelsize=tick_size, pad = l_pad)
    ax.tick_params(axis='both', which='minor', labelsize=tick_size, pad = l_pad)
    ax.legend(fontsize = legend_size, loc = 'upper left')
    ax.grid()
    plt.savefig('Figures/validation.pdf')

