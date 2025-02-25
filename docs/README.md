# MJQM simulator

Simulator for Multiserver Job Queuing Model (MJQM)

## Prerequisite

1. C++ toolchain, in particular `g++` with support for the `C++20` standard
2. CMake toolchain, version `3.16` or higher
3. Boost library, version `1.83` or higher
4. _[optional]_ Python3, for tests

## Build

To prepare and compile the project with `cmake`, use the following command from the project root directory:

```sh
./configure [--debug] [--clean] [--test] [--no-build]
```

This will create an executable named `<file>` for each configured `<file>.cpp` in the root directory.

The additional parameters work as such:

- `--debug` to build with debug symbols (useful for IDEs).
- `--clean` to remove the cmake directory before configuring the project, effectively doing a _full fresh restart_.
- `--test` to also run tests.
- `--no-build` to only configure the project without building it.

### Rebuild

If you change some code and want to rebuild the project, you can use the `rebuild` script:

```sh
./rebuild [--debug] [--clean]
```

The additional parameters work as such:

- `--debug` to rebuild with debug symbols (you need to have configured the project with the `--debug` parameter already).
- `--clean` to add the `--clean-first` parameter to cmake, that will remove all prebuilt symbols and objects before rebuilding the project, without doing a _full fresh restart_.

## Test

> **Note**: The test suite will be completely reworked soon.

To run the test suite, use the following command after configuration is done:

```sh
cmake --build . --preset test
```

This will run all the simulation tests for which results are available in the `test/expected` folder.

Their run parameters are configured in the `CMakeLists.txt` file.

## Run

To instructions for running the simulator, please refer to the [Run](./run.md) document.
