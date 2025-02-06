//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <chrono>
#include <fstream>
#include <iostream>
#include <mjqm-settings/toml_loader.h>
#include <mjqm-simulator/simulator.h>
#include <mjqm-simulator/stats.h>
#include <string>
#include <thread>
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
    std::string out_filename;
    std::string input_name(argv[1]);
    auto overrides = parse_overrides_from_args(argc, argv);
    auto conf_result = from_toml("Inputs/" + input_name + ".toml", overrides);
    if (conf_result->empty() || !conf_result->at(0).first) {
        std::cerr << "Error reading TOML file" << std::endl;
        return 1;
    }
    auto conf_to_run = conf_result->size();
    unsigned int classes = conf_result->at(0).second.get_sizes(sizes);
    out_filename = "Results/simulator_toml/overLambdas-nClasses" + std::to_string(classes) + "-N" +
        std::to_string(conf_result->at(0).second.cores) + "-Win" + std::to_string(0) + "-Exponential-" +
        conf_result->at(0).second.name + ".csv";
    for (const auto& conf: *conf_result) {
        std::cout << conf.second.toml << std::endl << std::endl;
    }
    std::ofstream outputFile(out_filename, std::ios::app);

    std::vector<ExperimentStats> experiments_stats(conf_to_run);
    std::vector<std::thread> threads(conf_to_run);

    for (int i = 0; i < conf_to_run; i++) {
        threads[i] = std::thread(run_simulation, std::ref(conf_result->at(i).second), std::ref(experiments_stats[i]));
    }
    for (int i = 0; i < conf_to_run; i++) {
        threads[i].join();
    }

    if (outputFile.tellp() == 0) {
        experiments_stats.at(0).add_headers(headers, sizes);
        // Write the headers to the CSV file
        for (const std::string& header : headers) {
            outputFile << header << ";";
        }
        outputFile << "\n";
    }

    for (int i = 0; i < conf_to_run; i++) {

        outputFile << conf_result->at(i).second.toml.at_path("arrival.rate").value<double>().value() << ";";
        outputFile << experiments_stats[i] << "\n";
    }

    // Close the file
    outputFile.close();

    return 0;
}
