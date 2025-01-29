# mjqm-simulator
Simulator for Multiserver Job Queuing Model (MJQM)

## Prerequisite
1. A working laptop
2. CMake toolchain
3. C++ toolchain

## Input files
We need two types of input files, both to be placed in the `Inputs` folder:
1. First file containing a list of job classes with format (server need, probability, average holding time). e.g. oneOrAll_N32_0.6.txt
2. Second file containing a list of arrival rates to be supplied to the simulator in format [xxx,.....,xxx]. Naming has to begin with arrRate-> e.g. arrRate_oneOrAll_N32_0.6.txt

## How-to
1. Prepare and compile the project with `cmake`.
   ```shell
   cmake . -DCMAKE_BUILD_TYPE=Release --fresh
   cmake --build . --clean-first -j 8
   ```
   Same as:
   ```shell
   ./configure
   ```
   This will create an executable named `<file>` for each `<file>.cpp` in the root directory.
2. Run the produced executable with parameters. e.g. ./simulator_smash oneOrAll_N32_0.6 32 1 exp 100000 10
   `./<file> [experiment name] [server cores] [scheduling policy] [service time distributions -> exp/par/det/uni/bpar/fre] [Number of events] [Number of repetitions])`
   The experiment name needs to be the same as the [input file name](#input-files) without the extension.
3. Results will be given in csv format in the `Results` folder, with files named after the parameters provided.
   The specific column we want is named "Queue Total" which represents the mean queue length.
