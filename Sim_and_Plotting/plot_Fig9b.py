#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Aug  5 09:31:21 2024

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
    asymptotes = [156.154, 36.9231, 148.462]
    #asymptotes = [160.0, 40.7692, 79.2308, 152.308, 262.895]
    ys = [0.5, 0.5, 0.5, 1, 0.5]
    xs = [0.1, 0.3, 0.3, 0.3, 0.4]
    ylims = [0.1, 10**3]
    locs = ['upper left', 'upper left', 'upper left']
elif cell == 'cellB':
    n = 2048
    nClasses =26
    #asymptotes = [1.53542, 3.3832, 4.1916, 4.65354, 4.88451]
    asymptotes = [1.53542, 3.3832, 4.65354, 4.07611, 0.727016]
    ys = [1.2, 4, 15, 40, 110]
    xs = [0.1, 0.3, 0.3, 0.3, 0.3]
    ylims = [0.1, 10**5]
    locs = ['upper left', 'upper left', 'lower left']
else:
    n = 1024
    nClasses = 26
    #asymptotes = [119.208, 8.49902, 17.785, 34.0354, 61.8933]
    asymptotes = [117.667, 6.17753, 15.4635, 31.7139, 59.5719]
    ys = [1.2, 4, 15, 40, 110]
    xs = [0.1, 0.3, 0.3, 0.3, 0.3]
    ylims = [0.1, 10**5]
    locs = ['upper left', 'upper left', 'upper left']
 
#combos = [[32, 1024], [16, 512], [8, 256], [4, 256]]
combos = [[32, 1024], [8, 256]]
#cols = ['peru', 'royalblue', 'crimson', 'purple', 'darkgreen']
#cols = ['peru', 'royalblue', 'purple']
#styles = ['solid', 'dotted', 'dashed', 'dashdot', (0, (3, 5, 1, 5, 1, 5))]

cols = ['peru', 'royalblue', 'purple', '#FF00FF', '#00FFFF']
styles = ['solid', 'dotted', 'dashdot', (0, (5, 10)), (0, (1, 1))]

filenames = []
labels = []
txts = []

filenames.append('Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-'+cell+f'-Sorted_{n}.csv')
txts.append(cell + '/' + cell + f'_Sorted_{n}')
labels.append('Original demands')
for combo in combos:
    fnm = 'Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-' +cell+f'-hyper-Ts{combo[0]}-Tb{combo[1]}-N{n}.csv'
    txt = cell + '/' + cell + f'_hyper-Ts{combo[0]}-Tb{combo[1]}-N{n}'
    l = r'$\mathcal{T}_s =$ '+ f'{combo[0]}, '+ r'$\mathcal{T}_b =$ ' + f'{combo[1]}'
    
    filenames.append(fnm)
    labels.append(l)
    txts.append(txt)
    
filenames.append('Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-'+cell+f'-hyper-LowerPowOfTwo-N{n}.csv')
filenames.append('Results/' + cell + f'/Exp/OverLambdas-nClasses{nClasses}-N{n}-Win1-Exponential-'+cell+f'-hyper-UpperPowOfTwo-N{n}.csv')

txts.append(cell + '/' + cell + f'_hyper-LowerPowOfTwo-N{n}')
txts.append(cell + '/' + cell + f'_hyper-UpperPowOfTwo-N{n}')

labels.append('Lower Pow. of 2')
labels.append('Upper Pow. of 2')

lambdas = [[] for file in filenames]    
blocked_cores = [[] for file in filenames]   
busy_cores = [[] for file in filenames]   

ps = load_probs(txts[0])
cores = [[] for file in filenames]
times = [[] for file in filenames]

for f in range(len(filenames)):
    cores[f], times[f] = load_times(txts[f])
    
  
for f in range(len(filenames)):
    
    with open(filenames[f]) as csv_file:
        df = pd.read_csv(filenames[f], delimiter = ';')
        Ts = list(set(cores[f]))
        Ts.sort()
        
        for index, row in df.iterrows():
       
            lambdas[f].append(float(row['Arrival Rate']))
            blocked_cores[f].append(float(row['Utilisation']))
    asymptotes[f] = lambdas[f][-1]
  
for f in range(0, len(filenames)):
    for i in range(0, len(lambdas[f])):
      
        summ_util = 0
        for t in range(len(cores[0])):
            if cores[0][t] <= cores[f][t]:
                summ_util += lambdas[f][i]*ps[t]*cores[0][t]*times[f][t]*(1/n)
            else:
                summ_util += lambdas[f][i]*ps[t]*cores[f][t]*times[f][t]*(1/n)
        busy_cores[f].append(summ_util)
   

plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = 70
fix, ax = plt.subplots(figsize=(35,30))        

ax.plot(lambdas[4][:lambdas[4].index(asymptotes[4])], blocked_cores[4][:lambdas[4].index(asymptotes[4])], color = cols[4], label = r'Upper Pow. of 2 - - Core Allocation', ls = 'solid', lw=12)
ax.plot(lambdas[4][:lambdas[4].index(asymptotes[4])], busy_cores[4][:lambdas[4].index(asymptotes[4])], color = cols[4], label = r'Upper Pow. of 2 - Core Utilisation', ls = 'dashed', lw=12) 
plt.axvline(x = asymptotes[2], color = cols[2], linestyle = 'dashdot', lw=8)


ax.plot(lambdas[3][:lambdas[3].index(asymptotes[3])], blocked_cores[3][:lambdas[3].index(asymptotes[3])], color = cols[3], label = fr'Lower Pow. of 2 - Core Allocation', ls = 'solid', lw=12)
ax.plot(lambdas[3][:lambdas[3].index(asymptotes[3])], busy_cores[3][:lambdas[3].index(asymptotes[3])], color = cols[3], label = fr'Lower Pow. of 2 - Core Utilisation', ls = 'dashed', lw=12) 
plt.axvline(x = asymptotes[3], color = cols[3], linestyle = 'dashdot', lw=8)

ax.plot(lambdas[2][:lambdas[2].index(asymptotes[2])], blocked_cores[2][:lambdas[2].index(asymptotes[2])], color = cols[2], label = r'$\mathcal{T}_s = $ '+f'{combos[1][0]}, ' + r'$\mathcal{T}_b = $ ' + f' {combos[1][1]} - Core Allocation', ls = 'solid', lw=12)
ax.plot(lambdas[2][:lambdas[2].index(asymptotes[2])], busy_cores[2][:lambdas[2].index(asymptotes[2])], color = cols[2], label = r'$\mathcal{T}_s = $ '+f'{combos[1][0]},  '+ r'$\mathcal{T}_b = $ ' + f'{combos[1][1]} - Core Utilisation', ls = 'dashed', lw=12) 
plt.axvline(x = asymptotes[2], color = cols[2], linestyle = 'dashdot', lw=8)

ax.plot(lambdas[1][:lambdas[1].index(asymptotes[1])], blocked_cores[1][:lambdas[1].index(asymptotes[1])], color = cols[1], label = r'$\mathcal{T}_s = $ '+f'{combos[0][0]},'+ r' $\mathcal{T}_b = $ ' + f'{combos[0][1]} - Core Allocation', ls = 'solid', lw=12)
ax.plot(lambdas[1][:lambdas[1].index(asymptotes[1])], busy_cores[1][:lambdas[1].index(asymptotes[1])], color = cols[1], label = r'$\mathcal{T}_s = $ '+f'{combos[0][0]},'+ r' $\mathcal{T}_b = $ ' + f'{combos[0][1]} - Core Utilisation', ls = 'dashed', lw=12)
plt.axvline(x = asymptotes[1], color = cols[1], linestyle = 'dashdot', lw=8)

ax.plot(lambdas[0][:lambdas[0].index(asymptotes[0])], blocked_cores[0][:lambdas[0].index(asymptotes[0])], color = cols[0], label = 'Original demands - Core Allocation', ls = 'solid', lw=12)
ax.plot(lambdas[0][:lambdas[0].index(asymptotes[0])], busy_cores[0][:lambdas[0].index(asymptotes[0])], color = cols[0], label = 'Original demands - Core Utilisation', ls = 'dashed', lw=12)
plt.axvline(x = asymptotes[0], color = cols[0], linestyle = 'dashdot', lw=8)

'''ax.plot(lambdas[0], blocked_cores[0], color = cols[0], label = 'Original demands - Core Allocation', ls = 'solid', lw=8)
ax.plot(lambdas[0], busy_cores[0], color = cols[0], label = 'Original demands - Core Utilization', ls = 'dashed', lw=8)
plt.axvline(x = asymptotes[0], color = cols[0], linestyle = 'dashdot', lw=8)


ax.plot(lambdas[1], blocked_cores[1], color = cols[1], label = fr'$T_s = {combos[0][0]},\ T_b = {combos[0][1]}$ - Core Allocation', ls = 'solid', lw=8)
ax.plot(lambdas[1], busy_cores[1], color = cols[1], label = fr'$T_s = {combos[0][0]},\ T_b = {combos[0][1]}$ - Core Utilization', ls = 'dashed', lw=8)
plt.axvline(x = asymptotes[1], color = cols[1], linestyle = 'dashdot', lw=8)


ax.plot(lambdas[2], blocked_cores[2], color = cols[2], label = fr'$T_s = {combos[1][0]},\ T_b = {combos[1][1]}$ - Core Allocation', ls = 'solid', lw=8)
ax.plot(lambdas[2], busy_cores[2], color = cols[2], label = fr'Aggregation $T_s = {combos[1][0]},\ T_b = {combos[1][1]}$ - Core Utilization', ls = 'dashed', lw=8) 
plt.axvline(x = asymptotes[2], color = cols[2], linestyle = 'dashdot', lw=8)'''



ax.set_xlabel(r"Arrival Rate $\quad [$s$^{-1}]$", fontsize=80)
ax.set_ylabel("Utilisation", fontsize=80)
ax.set_title("Avg. Util vs. Arrival Rate")
plt.xscale('log')
#plt.yscale('log')
ax.legend(loc=locs[0], fontsize=70)
ax.grid()
plt.savefig('Figures/utilAnalysis-' + cell + '.pdf')








