## Requirements
- C++14/17/20
- Python 3

## How to reproduce the plots

### Option 1: Only do the plotting from pre-saved simulation results
1. cd Only_Plotting
2. chmod +x script-figures.sh
3. ./script-figures.sh

The plots will be generated inside folder [Figures](Only_Plotting/Figures/)

### Option 2: Do all the simulations first, then do the plotting
1. cd Sim_and_Plotting
2. chmod +x script-figures.sh res_Fig3a.sh res_others.sh generate_all.sh
3. ./generate_all.sh

The simulation results will be generated inside foler [Results](Sim_and_Plotting/Results/), while the plots will be generated inside folder [Figures](Sim_and_Plotting/Figures/)
