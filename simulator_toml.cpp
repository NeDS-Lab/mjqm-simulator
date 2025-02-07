//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <fstream>
#include <iostream>
#include <mjqm-settings/toml_loader.h>
#include <mjqm-simulator/simulator.h>
#include <mjqm-simulator/stats.h>
#include <ranges>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

void run_simulation(const ExperimentConfig& conf,
                    ExperimentStats& stats // out
) {
    Simulator sim(conf);
    sim.reset_simulation();
    sim.reset_statistics();

    sim.simulate(conf.events, conf.repetitions);
    sim.produce_statistics(stats);
}

int main(int argc, char* argv[]) {
    std::vector<unsigned int> sizes;
    std::vector<std::string> headers{"Arrival Rate"};
    std::string input_name(argv[1]);
    auto overrides = parse_overrides_from_args(argc, argv);
    auto experiments = from_toml("Inputs/" + input_name + ".toml", overrides);
    if (experiments->empty() || !experiments->at(0).first) {
        std::cerr << "Error reading TOML file" << std::endl;
        return 1;
    }

    auto n_experiments = experiments->size();

    std::vector<ExperimentStats> experiments_stats(n_experiments);
    std::vector<std::thread> threads(n_experiments);

    std::unordered_map<std::string, std::ofstream> out_files;
    for (const auto& conf : std::views::values(*experiments)) {
        std::cout << conf << std::endl;
        std::cout << conf.toml << std::endl << std::endl;
        std::string out_filename = conf.output_filename();
        if (!out_files.contains(out_filename)) {
            out_files[out_filename] = std::ofstream(out_filename, std::ios::app);
            if (out_files[out_filename].tellp() == 0) {
                conf.get_sizes(sizes);
                experiments_stats.at(0).add_headers(headers, sizes);
                // Write the headers to the CSV file
                for (const std::string& header : headers) {
                    out_files[out_filename] << header << ";";
                }
                out_files[out_filename] << "\n";
            }
        }
    }

    for (int i = 0; i < n_experiments; i++) {
        threads[i] = std::thread(run_simulation, std::ref(experiments->at(i).second), std::ref(experiments_stats[i]));
    }
    for (int i = 0; i < n_experiments; i++) {
        threads[i].join();
    }

    for (int i = 0; i < n_experiments; i++) {
        std::string out_filename = experiments->at(i).second.output_filename();
        out_files[out_filename] << experiments->at(i).second.toml.at_path("arrival.rate").value<double>().value()
                                << ";";
        out_files[out_filename] << experiments_stats[i] << "\n";
    }

    for (auto& file : out_files | std::views::values) {
        file.close();
    }

    return 0;
}
