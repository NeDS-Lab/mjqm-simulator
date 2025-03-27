#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Nov 26 19:53:05 2023

@author: dilettaolliaro
"""

from matplotlib.patches import Rectangle
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib
import csv
import re

def load_probs(input_txt):
    
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

    # Sort the data based on the first column (ascending order)
    sorted_data = sorted(data, key=lambda x: x[0])
    

    # If you want to work with the sorted lists separately
    T, p, mus = zip(*sorted_data)
    T, p, mus = list(T), list(p), list(mus)
    
    p = np.array(p)
  
    return p

def load_times(input_txt):
    
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

    # Sort the data based on the first column (ascending order)
    sorted_data = sorted(data, key=lambda x: x[0])
    

    # If you want to work with the sorted lists separately
    T, p, mus = zip(*sorted_data)
    T, p, mus = list(T), list(p), list(mus)
    
    p = np.array(p)
  
    return T, mus

# Function to extract T values from the header
def extract_t_values(csv_file_path):
    # Read the CSV file into a DataFrame, specifying that the delimiter is a tab ('\t')
    df = pd.read_csv(csv_file_path, delimiter= ';')

    # Extract the column names (header) excluding the first column 'W'
    header = df.columns[1:]

    # Extract and parse the T values using list comprehension
    t_values = list(set([int(re.search(r'\d+', column).group()) for column in header[1:] if column.startswith('T')]))

    # Sort the T values in ascending order
    t_values.sort()

    return t_values

cell = 'cellB'
lims = []
if cell == 'cellA':
    n = 3072
    nClasses = 29
    asymptotes = [156.154, 36.9231]
    #asymptotes = [160.0, 40.7692, 79.2308, 152.308, 262.895]
    ys = [0.5, 0.5, 0.5, 1, 0.5]
    xs = [0.1, 0.3, 0.3, 0.3, 0.4]
    ylims = [0, 10**3]
    locs = ['upper left', 'upper left', 'upper left']
elif cell == 'cellB':
    n = 2048
    nClasses =26
    asymptotes = [1.53542, 3.3832, 4.76903]
    ylims = [1, 35000]
    locs = ['upper left', 'upper left', 'lower left']

combos = [[32, 1024]]
 
cols = ['peru', 'royalblue', 'crimson', 'purple', 'darkgreen', 'black']
styles = ['solid', 'dotted', 'dashed', 'dashdot', (0, (3, 5, 1, 5, 1, 5)), 'solid']

filenames = []
labels = []
txts = []

# arrRate_varServiceTimes_2048
filenames.append('Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-'+cell+f'-Sorted_{n}.csv')
labels.append('Original demands')
txts.append(cell + '/' + cell + f'_Sorted_{n}')

for combo in combos:
    fnm = 'Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-' +cell+f'-hyper-Ts{combo[0]}-Tb{combo[1]}-N{n}.csv'
    txt = cell + '/' + cell + f'_hyper-Ts{combo[0]}-Tb{combo[1]}-N{n}'
    l = r'\ $\mathcal{T}_s = $ ' + f'{combo[0]}, ' + r' $\mathcal{T}_b = $ ' + f'{combo[1]}'
    
    filenames.append(fnm)
    labels.append(l)
    txts.append(txt)

filenames.append('Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-'+cell+f'-VarServiceTimes-N{n}.csv')
labels.append(r'Reduced $\sigma_S$')
txts.append(cell + '/' + cell + f'_VarServiceTimes-N{n}')



lambdas = [[] for file in filenames]    

systems_tot = [[] for file in filenames]
systems_small = [[] for file in filenames]
systems_big = [[] for file in filenames]
   
respTimes_tot = [[] for file in filenames]
respTimes_small = [[] for file in filenames]
respTimes_big = [[] for file in filenames]


ps = load_probs(txts[0])
cores = [[] for file in filenames]
times = [[] for file in filenames]

for f in range(len(filenames)):
    cores[f], times[f] = load_times(txts[f])
    
  
for f in range(len(filenames)):
    flag = 1
    with open(filenames[f]) as csv_file:
        df = pd.read_csv(filenames[f], delimiter = ';')
        Ts = list(set(cores[f]))
        Ts.sort()
        print(Ts)

        
        warns = []
        summ_sys = 0
       
         
        for index, row in df.iterrows():
        
            if len(Ts) > 2:    
                
                summ_sys = 0
                
                for i in range(nClasses):
                    summ_sys += float(row[f'T{Ts[i]} System'])
                    warns.append(int(row[f'T{Ts[i]} Stability Check']))
              
    
            else:
             
                summ_sys = float(row[f'T{min(Ts)} System'])
                warns.append(int(row[f'T{min(Ts)} Stability Check']))
                
                for i in range(1, nClasses-1):
                    summ_sys += float(row[f'T{min(Ts)} System.{i}'])
                    warns.append(int(row[f'T{min(Ts)} Stability Check.{i}']))
        
                summ_sys += float(row[f'T{max(Ts)} System'])
                warns.append(int(row[f'T{max(Ts)} Stability Check']))
              
            lambdas[f].append(float(row['Arrival Rate']))
            systems_tot[f].append(summ_sys)
            systems_small[f].append(float(row[f'T{min(Ts)} System']))
            systems_big[f].append(float(row[f'T{max(Ts)} System']))
                            
                     
for f in range(0, len(filenames)):
    for i in range(0, len(lambdas[f])):
        respTimes_tot[f].append(systems_tot[f][i]/lambdas[f][i])
        respTimes_small[f].append(systems_small[f][i]/(lambdas[f][i]*ps[0]))
        respTimes_big[f].append(systems_big[f][i]/(lambdas[f][i]*ps[-1]))
        
    #lims.append(lambdas[f].index(asymptotes[f]))


   
plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = 70
fix, ax = plt.subplots(figsize=(35,30))        

for f in range(len(filenames)):  
    prelim = 0#19 if (f == 0 and cell == 'cellB') else 0      
    ax.plot(lambdas[f], respTimes_tot[f], color = cols[f], label = labels[f], ls = styles[f], lw=12)
    plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dashdot', lw=8)
    


ax.set_xlabel("Arrival Rate $\quad [$s$^{-1}]$", fontsize=80)
ax.set_ylabel("Avg. Response Time $\quad [$s$]$", fontsize=80)
ax.set_title("Avg. Total Response Time vs. Arrival Rate")
plt.xscale('log')
plt.ylim(ylims[0], ylims[1])
plt.yscale('log')
ax.legend(loc=locs[0], fontsize = 80)
ax.grid()
plt.savefig(f'Figures/lambdasVsTotRespTime-UniVar-Ts_{combos[0][0]}-Tb_{combos[0][1]}-' + cell + '.pdf')


'''plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = 50
fix, ax = plt.subplots(figsize=(30,30))        

for f in range(len(filenames)):        
    ax.plot(lambdas[f][prelim:lims[f]], respTimes_small[f][prelim:lims[f]], color = cols[f], label = labels[f], ls = styles[f], lw=8)
    plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dashdot', lw=3)
    

ax.set_xlabel("Arrival Rate")
ax.set_ylabel("Avg. Response Time")
ax.set_title("Avg. Response Time of Smallest Class vs. Arrival Rate")
plt.xscale('log')
plt.ylim(ylims[0], ylims[1])
plt.yscale('log')
ax.legend(loc=locs[0])
ax.grid()
plt.savefig('lambdasVsSmallRespTime-' + cell + '.pdf')



plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = 50
fix, ax = plt.subplots(figsize=(30,30))        

for f in range(len(filenames)):        
    ax.plot(lambdas[f][prelim:lims[f]], respTimes_big[f][prelim:lims[f]], color = cols[f], label = labels[f], ls = styles[f], lw=8)
    plt.axvline(x = asymptotes[f], color = cols[f], linestyle = 'dashdot', lw=3)
    

ax.set_xlabel("Arrival Rate")
ax.set_ylabel("Avg. Response Time")
ax.set_title("Avg. Response Time of Biggest Class vs. Arrival Rate")
plt.xscale('log')
plt.ylim(ylims[0], ylims[1])
plt.yscale('log')
ax.legend(loc=locs[2])
ax.grid()
plt.savefig('lambdasVsBigRespTime-' + cell + '.pdf')'''





