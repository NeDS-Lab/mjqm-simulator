# MJQM simulator
Simulator for Multiserver Job Queuing Model (MJQM)

## Prerequisite
1. A working laptop
2. CMake toolchain
3. C++ toolchain

## Input files
We need two types of input files, both to be placed in the [Inputs](./Inputs) folder:
1. First file containing a list of job classes with format (server need, probability, average holding time) -> e.g. [oneOrAll_N32_0.6.txt](./Inputs/oneOrAll_N32_0.6.txt)
2. Second file containing a list of arrival rates to be supplied to the simulator in format `[xxx,.....,xxx]`. Naming has to begin with arrRate -> e.g. [arrRate_oneOrAll_N32_0.6_W1.txt](./Inputs/arrRate_oneOrAll_N32_0.6_W1.txt)

## How-to
1. Prepare and compile the project with `cmake`.
   ```shell
   cmake --preset default --fresh
   cmake --build . --preset default
   ```
   Same as:
   ```shell
   ./configure
   ```
   This will create an executable named `<file>` for each `<file>.cpp` in the build directory.
2. Run the produced executable with parameters. e.g. `./simulator_smash oneOrAll_N32_0.6 32 1 exp 100000 10`
   ```shell
   ./<file> [experiment name] [server cores] [scheduling policy] [service time distributions -> exp/par/det/uni/bpar/fre] [Number of events] [Number of repetitions])
   ```
   The experiment name needs to be the same as the [input file name](#input-files) described above without the extension.
3. Results will be given in csv format in the `Results` folder, with files named after the parameters provided.
   The specific column we usually want is named "Queue Total" which represents the mean queue length.

## Test
To run the test suite, use the following command, or `make test_all` in the build directory:
```shell
cmake --build . --preset test
```
This will run all the simulation tests for which results are available in the [test/expected](./test/expected) folder.

Their run parameters are configured in the [CMakeLists.txt](./CMakeLists.txt) file.