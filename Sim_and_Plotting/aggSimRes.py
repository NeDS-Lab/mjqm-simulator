#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Aug  2 13:36:29 2024

@author: dilettaolliaro
"""

import csv
import pandas as pd

mubs = [0.01, 0.0188, 0.0192, 0.0199, 0.025, 0.03, 0.0355, 0.042, 0.045, 0.0499,
    0.053, 0.058, 0.061, 0.0669, 0.072, 0.08, 0.085, 0.091, 0.1262, 0.2378,
    0.24, 0.245, 0.25, 0.255, 0.26, 0.27, 0.28, 0.285, 0.2860, 0.2862,
    0.2867, 0.2883, 0.29, 0.30, 0.35, 0.4, 0.42, 0.4481, 0.4781, 0.4881,
    0.5, 0.5281, 0.5332, 0.5522, 0.5721, 0.5821, 0.5871, 0.5921, 0.6021,
    0.6121, 0.6221, 0.6321, 0.6421, 0.6521, 0.6621, 0.6721, 0.7221, 0.7721,
    0.8221, 0.8321, 0.8446, 1, 1.5918, 1.62, 1.65, 1.7, 1.89, 2.1, 3, 3.5,
    4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 10, 12, 14, 16, 18, 20, 22,
    24, 26, 28, 29, 30, 35, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 59, 60,
    61, 62, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
    86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
    103, 104, 105, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220,
    230, 240, 250, 350, 450, 500]

# Range of exID integers
exID_range = [x for x in range(0, 162)]

# Name of the output CSV file
output_csv = 'Results/Validation/simRes.csv'

# List to store the rows for the CSV file
rows = []

for exID in exID_range:
    # Construct the filename
    filename = f"Results/resDisagg/OverLambdas-nClasses2-N200-Win1-Exponential-cellX-exID{exID}_200.csv"
    with open(filename) as csv_file:

        df = pd.read_csv(filename, delimiter = ';')

        for index, row in df.iterrows():
             
             wasted_servers_value = float(row['Wasted Servers'])
             
    rows.append([mubs[exID], wasted_servers_value])
    print(exID)
   
# Write the rows to the output CSV file
with open(output_csv, mode='w', newline='') as file:
    writer = csv.writer(file)
    # Write the header
    writer.writerow(['Big Class Rate', 'Wasted Servers'])
    # Write the rows
    writer.writerows(rows)

print(f"Output written to {output_csv}")
