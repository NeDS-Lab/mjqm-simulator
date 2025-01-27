//
// Created by mccio on 26/01/25.
//

#include <iostream>
#include "settings/toml_loader.hpp"

// watchexec --clear --timings --env appname=toml_loader_test --debounce 5s --restart --exts h,hpp,cpp,toml -- "make run"
int main()
{
    std::string filename = "oneOrAll_N32_0.6.toml";
    std::cout << "Reading TOML file: " << filename << std::endl;
    ExperimentConfig config;
    from_toml(filename, config);
    std::cout << "Experiment name: " << config.name << std::endl;
    std::cout << "Number of events: " << config.events << std::endl;
    std::cout << "Number of repetitions: " << config.repetitions << std::endl;
    std::cout << "Number of system cores: " << config.cores << std::endl;
    std::cout << "Policy: " << config.policy << std::endl;
    std::cout << "Classes: " << config.classes.size() << std::endl;
    for (const auto& [name, cores, arrival_distribution, service_distribution] : config.classes)
    {
        std::cout << "- Name: " << name << std::endl;
        std::cout << "  Cores: " << cores << std::endl;
        std::cout << "  Arrival: " << arrival_distribution->to_string() << std::endl;
        std::cout << "  Service: " << service_distribution->to_string() << std::endl;
    }
}