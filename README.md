# mjqm-simulator
Simulator for Multiserver Job Queuing Model (MJQM)

## Prerequisite
1. C++
2. A working laptop

## Input files
We need two types of input files:
1. First file containing a list of job classes with format (server need, probability, average holding time). e.g. oneOrAll_N32_0.6.txt
2. Second file containing a list of arrival rates to be supplied to the simulator in format [xxx,.....,xxx]. Naming has to begin with arrRate-> e.g. arrRate_oneOrAll_N32_0.6_W4.txt

n.b. The code is hard-coded to run simulations automatically for window size 1 until 6 (line 909), thus Input files no.2 end with _Wx (where x is window size)

## How-to
1. Compile the .cpp file (A sample of an executable is provided)
2. Run the produced executable with parameters. e.g. ./smash_gr.out 32 exp oneOrAll_N32_0.6 1000000 20 (./your-file.out [server need of big jobs] [service time distributions -> exp/par/det/uni/bpar/fre] [Input files no.1] [Number of events] [Number of repetitions])
3. Results will be given in csv format. The specific column we want is named "Queue Total" which represents the mean queue length.
