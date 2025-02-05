//
// Created by Marco Ciotola on 26/01/25.
//

#include <iostream>
#include <map>
#include <mjqm-settings/toml_loader.h>
#include <ranges>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    std::string_view filename = argc > 1 ? argv[1] : "../oneOrAll_N32_0.6.toml";
    std::cout << "Reading TOML file: " << filename << std::endl;

    std::map<std::string, std::vector<std::string>> map = {
        {"simulation.smash.window", {"6", "2", "3", "4", "5"}}
    };

    auto experiments = from_toml(filename, map);
    int i = 0;
    for (const auto& [success, config] : *experiments) {
        std::cout << ++i << ": " << success << ": " << std::endl;
        std::cout << config;
        std::cout << std::endl;
    }
    //
    // ExperimentConfig config;
    // bool success = from_toml(filename, config);
    // std::cout << config << std::endl;
    // if (!success) {
    //     std::cerr << "Error reading TOML file" << std::endl;
    //     return 1;
    // }
}