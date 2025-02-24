# MJQM simulator

Simulator for Multiserver Job Queuing Model (MJQM)

## Prerequisite

1. A working laptop
2. CMake toolchain
3. C++ toolchain

## Build

To prepare and compile the project with `cmake`, use the following command from the project root directory:

```shell
./configure
```

That is the same as running:

```shell
cmake --preset default --fresh
cmake --build . --preset default
```

This will create an executable named `<file>` for each `<file>.cpp` in the root directory.

### Build parameters
You can provide additional parameters to the `configure` script, such as:
- `--debug` to build with debug symbols
- `--clean` to remove the cmake directory before configuring the project, effectively doing a _full fresh restart_.
- `--test` to also run tests.
- `--no-build` to only configure the project without building it.

### Rebuild
If you change some code and want to rebuild the project, you can use the `rebuild` script:

```shell
./rebuild
```

You can also provide some parameters to the `rebuild` script, such as:
- `--debug` to rebuild with debug symbols
- `--clean` to add the `--clean-first` parameter to cmake, that will remove all prebuilt symbols and objects before rebuilding the project, without doing a _full fresh restart_.

## Test

> **Note**: The test suite will be completely reworked soon.

To run the test suite, use the following command after configuration is done:

```shell
cmake --build . --preset test
```

This will run all the simulation tests for which results are available in the [test/expected](./test/expected) folder.

Their run parameters are configured in the [CMakeLists.txt](./CMakeLists.txt) file.

## Run

To instructions for running the simulator, please refer to the [run.md](./docs/run.md) document.
