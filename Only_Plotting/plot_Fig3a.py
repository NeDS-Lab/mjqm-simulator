#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Nov 17 19:46:49 2023

@author: andreamarin
"""
import re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
from matplotlib import rc
from matplotlib.patches import Rectangle

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


plotname = 'synthetic'

file0 = 'Results/sinthethicExp/twoClasses-N100-nClasses2-Exponential.csv'
label0 = 'Exponential'
file1 = 'Results/sinthethicExp/fourClasses-N100-nClasses4-Exponential.csv'
label1 = 'Hyperexponential'

file2 = 'Results/sinthethicExp/twoClasses-N100-nClasses2-Deterministic.csv'
label2 = 'Deterministic - 2 Classes'
file3 = 'Results/sinthethicExp/fourClasses-N100-nClasses4-Deterministic.csv'
label3 = 'Deterministic - 4 Classes'

file4 = 'Results/sinthethicExp/twoClasses-N100-nClasses2-BoundedPareto.csv'
label4 = 'BoundedPareto - 2 Classes'
file5 = 'Results/sinthethicExp/fourClasses-N100-nClasses4-BoundedPareto.csv'
label5 = 'BoundedPareto - 4 Classes'

nClasses = [2, 4]

# ORDER
# Exp HyperExp Det2 Det4 bPar2 bPar4
filenames = [file0, file1, file2, file3, file4, file5]
labels = [label0, label1, label2, label3, label4, label5]
cols = ["green", "green", "orange", "orange", 'blue', 'blue']
greys = ['#f0f0f0', '#dcdcdc', '#c0c0c0', '#a9a9a9', '#808080', '#696969']
stabs = ['#ffcccc', '#ff9999', '#ff6666', '#ff3333', '#ff0000', '#cc0000']
styles = ["solid", "dotted"]

lims = [0.58, 0.553, 0.634332, 0.568713, 0.612459, 0.557776, 1.1]


systems_tot = [[] for i in range(len(filenames))]
xvalues = [[] for i in range(len(filenames))]
respTimes_tot = [[] for i in range(len(filenames))]
lambdas = [[] for i in range(len(filenames))]

for f in range(len(filenames)):
   
    with open(filenames[f]) as csv_file:
        df = pd.read_csv(filenames[f], delimiter = ';')
        Ts = list(set(extract_t_values(filenames[f])))
        print(Ts)
        
        summ_sys = 0
        summ_th = 0
      
        for index, row in df.iterrows():
                  
            xvalues[f].append(float(row['Utilisation']))
            lambdas[f].append(float(row['Arrival Rate']))
            respTimes_tot[f].append(float(row['RespTime Total']))

        
plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = 70
fix, ax = plt.subplots(figsize=(40,26))  

for i in range(len(filenames)):
    ax.plot(xvalues[i], respTimes_tot[i], color = cols[i], label  = labels[i], linestyle = styles[i%2], lw=15)


ax.add_patch(Rectangle((0.6299, 0), 0.634332, 10000, facecolor="grey"))
ax.add_patch(Rectangle((0.553, 0), 0.045, 10000, facecolor="lightgrey")) 

'''ax.add_patch(Rectangle((0.58, 0), 0.6, 10000, facecolor="grey"))
ax.add_patch(Rectangle((0.553, 0), 0.045, 10000, facecolor="lightgrey"))

plt.axvline(x = 0.553, color = 'red', label = 'Hyperexponential stability', linestyle = 'dashdot', lw = 8)
plt.axvline(x = 0.6, color = 'black', label = 'Exponential stability', linestyle = 'dashdot', lw = 8)'''


plt.yscale('log')
#plt.xscale('log')
plt.xlim([1/100,1.1])
#plt.xlim([1/10000, 6])
plt.ylim([1,10000])
plt.grid()
ax.legend(loc = 'upper left', fontsize=70)
plt.savefig('Figures/2vs4Classes.pdf')
#plt.savefig('xlambdas-2vs4Classes.pdf')
