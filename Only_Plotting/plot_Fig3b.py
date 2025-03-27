#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Nov 17 19:46:49 2023

@author: andreamarin
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rc
from matplotlib.patches import Rectangle
import matplotlib 

def compute_pk(rho, mu, sigma2):
    l = rho * mu
    resp = (rho + l*mu*sigma2) / (2*(mu-l))+1/mu
    return resp

plt.figure(dpi=1200)
plt.rc('font',**{'family':'serif','serif':['Palatino']})
plt.rc('text', usetex=True)
matplotlib.rcParams['font.size'] = 70
fix, ax = plt.subplots(figsize=(40,26))  

xs = list(np.linspace(0, 0.98, 80)) + list(np.linspace(0.981, 0.999, 10))
exp = [compute_pk(x, 1, 1) for x in xs]
hexp = [compute_pk(x, 1, 5) for x in xs]
hhexp = [compute_pk(x, 1, 100) for x in xs]
#fix, ax = plt.subplots()
ax.plot(xs, exp, color="green", label  = "$\\sigma^2=1$", linestyle='solid', lw=12)
ax.plot(xs, hexp, color="blue",  label = "$\\sigma^2=5$", linestyle = 'dotted', lw=12)
ax.plot(xs, hhexp, color="orange",  label = "$\\sigma^2=100$", linestyle = 'dashed', lw=12)
ax.set_xlabel('$\\rho$', fontsize = 90)
ax.set_ylabel(r"Expected response time $\quad [$s$]$", fontsize = 90)
ax.set_title("M/G/1: Expected response time vs. offered load")
ax.add_patch(Rectangle((1, 0), 1, 10000, facecolor="lightgrey"))
plt.yscale('log')
#plt.xscale('log')
plt.xlim([1/100,1.1])
plt.ylim([1,10000])
plt.axvline(x = 1, color = 'red', label = 'Stability', linestyle = 'dashdot', lw=8)
plt.grid()
ax.legend(fontsize = 80)
plt.savefig('Figures/mg1.pdf')
