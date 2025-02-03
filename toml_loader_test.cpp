//
// Created by mccio on 26/01/25.
//

#include <iostream>
#include "settings/toml_loader.h"
#include <ranges>

// watchexec --clear --timings --env appname=toml_loader_test --debounce 5s --restart --exts h,hpp,cpp,toml -- "make run"
int main()
{
    std::string_view filename = "../oneOrAll_N32_0.6.toml";
    std::cout << "Reading TOML file: " << filename << std::endl;
    ExperimentConfig config;
    bool success = from_toml(filename, config);
    std::cout << config << std::endl;
    if (!success) {
        std::cerr << "Error reading TOML file" << std::endl;
        return 1;
    }
}