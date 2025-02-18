//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <mjqm-settings/toml_loader.h>
#include <mjqm-simulator/experiment_stats.h>
#include <mjqm-simulator/simulator.h>

void run_simulation(ExperimentConfig& conf) {
    Simulator sim(conf);
    sim.reset_simulation();
    sim.reset_statistics();

    sim.simulate(conf.events, conf.repetitions);
    sim.produce_statistics(conf.stats);
}

int main(int argc, char* argv[]) {
    std::string input_name(argv[1]);
    auto overrides = parse_overrides_from_args(argc, argv);
    auto experiments = from_toml("Inputs/" + input_name + ".toml", overrides);
    if (experiments->empty()) {
        std::cerr << "The provided identifier doesn't generate any configuration" << std::endl;
        return 1;
    }

    size_t n_experiments = experiments->size();

    for (int i = 0; i < n_experiments; ++i) {
        if (!experiments->at(i).first) {
            std::cerr << "Error reading TOML file" << std::endl;
            return 1;
        }
    }

    std::vector<std::thread> threads(n_experiments);

    std::unordered_map<std::string, std::ofstream> out_files;
    for (int i = 0; i < n_experiments; ++i) {
        const auto& conf = experiments->at(i).second;
        std::cout << conf << std::endl;
        std::string out_filename = conf.output_filename();
        if (!out_files.contains(out_filename)) {
            out_files[out_filename] = std::ofstream(out_filename, std::ios::app);
        }
    }
    for (int i = 0; i < n_experiments; ++i) {
        threads[i] = std::thread(run_simulation, std::ref(experiments->at(i).second));
    }
    for (int i = 0; i < n_experiments; ++i) {
        threads[i].join();
    }

    for (int i = 0; i < n_experiments; ++i) {
        const auto& conf = experiments->at(i).second;
        std::string out_filename = conf.output_filename();
        if (out_files[out_filename].tellp() == 0) {
            std::vector<unsigned int> sizes;
            std::vector<std::string> headers{};
            conf.stats.add_headers(headers);
            // Write the headers to the CSV file
            for (const std::string& header : headers) {
                out_files[out_filename] << header << ";";
            }
            out_files[out_filename] << "\n";
        }
        // out_files[out_filename] << conf.toml.at_path("arrival.rate").value<double>().value() << ";";
        out_files[out_filename] << conf.stats << "\n";
    }

    for (auto& file : out_files | std::views::values) {
        file.close();
    }

    return 0;
}
