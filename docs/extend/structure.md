# Project structure

## Folder organisation

```treeview
mjqm-simulator
├── docs
├── Inputs
│   └── {your_and_default_input_files...}
├── libs
│   ├── math
│   │   ├── include
│   │   │   ├── mjqm-math
│   │   │   │   ├── confidence_intervals.h
│   │   │   │   ├── sampler.h
│   │   │   │   └── samplers.h
│   │   │   └── mjqm-samplers
│   │   │       └── {specific_sampler...}.hpp
│   │   ├── src
│   │   │   └── mjqm-math
│   │   │       └── confidence_intervals.cpp
│   │   └── CMakeLists.txt
│   ├── policy
│   │   ├── include
│   │   │   └── mjqm-policy
│   │   │       ├── policies.h
│   │   │       ├── policy.h
│   │   │       └── {SpecificPolicy...}.h
│   │   ├── src
│   │   │   └── mjqm-policy
│   │   │       └── {SpecificPolicy...}.cpp
│   │   └── CMakeLists.txt
│   ├── simulator
│   │   ├── include
│   │   │   ├── mjqm-settings
│   │   │   │   ├── loader.hpp
│   │   │   │   ├── toml_distributions_loaders.h
│   │   │   │   ├── toml_loader.h
│   │   │   │   ├── toml_overrides.h
│   │   │   │   ├── toml_policies_loaders.h
│   │   │   │   └── toml_utils.h
│   │   │   └── mjqm-simulator
│   │   │       ├── experiment.h
│   │   │       ├── experiment_stats.h
│   │   │       ├── simulator.h
│   │   │       └── stats.h
│   │   ├── src
│   │   │   ├── mjqm-settings
│   │   │   │   ├── toml_distributions_loaders.cpp
│   │   │   │   ├── toml_loader.cpp
│   │   │   │   ├── toml_overrides.cpp
│   │   │   │   ├── toml_policies_loaders.cpp
│   │   │   │   └── toml_utils.cpp
│   │   │   └── mjqm-simulator
│   │   │       └── experiment_stats.cpp
│   │   └── CMakeLists.txt
│   └── utils
│       └── include
│           └── mjqm-utils
│               └── string.hpp
├── scripts
│   ├── convert_conf.py
│   ├── ensure_same_results.py
│   └── select-g++.sh
├── test
│   └── expected
│       └── {expected-output-files...}
├── CMakeLists.txt
├── CMakePresets.json
├── configure
├── README.md
├── rebuild
├── simula
├── simulator_smash.cpp
├── simulator_toml.cpp
└── toml_loader_test.cpp
```


The project is organized in the following way:

- `docs/`: Contains the documentation of the project.
- `Inputs/`: Contains the input files for the simulator. In the repository, only the input files for the tests are included.
- `libs/`: Contains the high level libraries used in the project.
    Each high level library has its own folder with the following structure:
    - `CMakeLists.txt`: Contains the CMake configuration for the library.
    - `include/`: Contains the header files of the library.
        This folder is organized in subfolders for each logical module of the library.
        The whole `include/` folder is included in the root `CMakeLists.txt`, so the headers are available to the whole project via names as `mjqm-{logical_module}/...`. This achieves explicit separation of our code from external libraries.
    - `src/`: Contains the source files of the library.
        Usually, each source file should be included in the `CMakeLists.txt` of the library.
- `libs/math/`: Contains the math code, including the confidence intervals and the samplers.
    Each sampler has its own header file that directly define the implementation.
    The `samplers.h` file includes all the samplers headers files for easier inclusion.
- `libs/policy/`: Contains the specific policies used in the simulator.
    Each policy has its own header and source file. The latter should be included in the `CMakeLists.txt` `policies` target.
    The `policies.h` file includes all the policies headers files for easier inclusion.
- `libs/simulator/`: Contains the actual simulator code and its settings loader.
    For a cleaner organization, the loaders for distributions and policies are separated.
- `libs/utils/`: Contains some quick header-only utilities.
- `scripts/`: Contains some scripts for solving small tasks running the project.
    - `convert_conf.py`: Converts the configuration files from the two-file logic to the TOML format.
    - `ensure_same_results.py`: Checks if the results of two simulations are the same.
- `test/expected/`: Contains the expected output files for the tests.
