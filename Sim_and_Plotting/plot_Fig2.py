#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Oct 26 08:45:25 2023

@author: dilettaolliaro
"""

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import scipy.special as sc
import math as m

def harmonic(n):
    result = 0
    for i in range(1, n + 1):
        result += 1 / i
    return result

def etbb_exp(taub, taus, ps, pb, N):
   beta = sc.betainc(N+1, 0.00000001, ps)*sc.beta(N+1, 0.00000001)
   a = taus*(((ps**N)/N)+beta+m.log(pb))
   b = (taus*(ps**N))/(N*pb)
   return taub - a + b


    
def etbb_det(taub, taus, ps, pb, N):
    num = taus*ps
    den = 1-(ps**N)
    return taub+(num/den)

#fig = '2c'  #'2bc'
for fig in ['2a','2b','2c']:
    ts = 1
    pbs = np.linspace(0.0001, 0.999, 5000)
    
    cols = ['#5ec962', '#3b528b', 'darkorange']
    styles = ['solid', 'dashed', 'dashdoth']
    
    plt.figure(dpi=1200)
    plt.rc('font',**{'family':'serif','serif':['Palatino']})
    plt.rc('text', usetex=True)
    matplotlib.rcParams['font.size'] = 70
    fix, ax = plt.subplots(figsize=(35,30))
    
    
    if fig == '2a':
        
        Ns = [256]
        tbs = [0.1, 1, 10]
        
        for n in Ns:
            
            for i in range(len(tbs)):
                l_max_det = []
                l_max_exp = []
                for pb in pbs:
                    ps = 1-pb
                    l_max_det.append(1/(etbb_det(tbs[i], ts, ps, pb, n)*pb))
                    l_max_exp.append(1/(etbb_exp(tbs[i], ts, ps, pb, n)*pb))
            
                ax.plot(pbs, l_max_det, color=cols[i], label=r'Det - $\tau_b$:' + f' {tbs[i]}', ls='solid', lw=8)
                ax.plot(pbs, l_max_exp, color=cols[i], label=r'Exp - $\tau_b$:' + f' {tbs[i]}', ls='dashed', lw=8)
            
            ax.set_xlabel(r'$p_b$', fontsize=130)
            ax.set_ylabel(r'$\lambda_{max}\quad [$s$^{-1}]$', fontsize=130)
            plt.yscale('log')
            #plt.ylim(1, 300)
            ax.set_title(r'Maximum Arrival Rate vs. Probability of Big Jobs')
            #ax.set_title(r'$\lambda_{max}$ vs. $p_b$', fontsize=50)
            ax.legend(fontsize = 70)
            ax.grid()
            plt.savefig(f'Figures/lambdaMax-N{n}'+'.pdf', format="pdf", bbox_inches="tight")
            #plt.show()
            #plt.close(fix)
    
    else:
        
        Ns = [64, 256]
        if fig == '2b':
            tbs = [0.1]
        elif fig == '2c':
            tbs = [1]
            
        for x in range(len(Ns)):
            for i in range(len(tbs)):
                l_max_det = [[] for n in Ns]
                l_max_exp = [[] for n in Ns]
                for pb in pbs:
                    ps = 1-pb
                    l_max_det[x].append(1/(etbb_det(tbs[i], ts, ps, pb, Ns[x])*pb))
                    l_max_exp[x].append(1/(etbb_exp(tbs[i], ts, ps, pb, Ns[x])*pb))
            
                ax.plot(pbs, l_max_det[x], color=cols[x], label=r'Det - $\mathcal{N}$:' + f' {Ns[x]}', ls='solid', lw=8)
                ax.plot(pbs, l_max_exp[x], color=cols[x], label=r'Exp - $\mathcal{N}$:' + f' {Ns[x]}', ls='dashed', lw=8)
    
        ax.set_xlabel(r'$p_b$', fontsize=130)
        ax.set_ylabel(r'$\lambda_{max}\quad [$s$^{-1}]$', fontsize=130)
        plt.yscale('log')
        plt.xscale('log')
        plt.ylim(0.8, 400)
        #plt.xlim(0, 0.2)
        ax.set_title(r'Maximum Arrival Rate vs. Probability of Big Jobs')
        #ax.set_title(r'$\lambda_{max}$ vs. $p_b$', fontsize=50)
        ax.legend(fontsize = 70)
        ax.grid()
        tstr = str(tbs[0]).replace('.', '')
        plt.savefig(f'Figures/lambdaMax-taub_{tstr}'+'.pdf', format="pdf", bbox_inches="tight")
        #plt.show()
        #plt.close(fix)
        
        
