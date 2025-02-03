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
    std::vector<double> arr_rate;
    std::vector<std::string> headers{"Arrival Rate"};
    std::string out_filename;
    std::vector<ExperimentConfig> ex(1);
    ExperimentConfig conf;
    std::string input_name(argv[1]);
    if (!from_toml("Inputs/" + input_name + ".toml", conf)) {
        std::cerr << "Error reading TOML file" << std::endl;
        return 1;
    }
    std::cout << conf;
    if (conf.name != input_name) {
        std::cerr << "Warning: Experiment name (" << conf.name << ") does not match the TOML file name (" << input_name
                  << "). The first will be used." << std::endl;
    }
    unsigned int classes = conf.get_sizes(sizes);
    out_filename = "Results/simulator_smash/overLambdas-nClasses" + std::to_string(classes) + "-N" +
        std::to_string(conf.cores) + "-Win" + std::to_string(1) + "-Exponential-" + conf.name + "-toml.csv";
    std::ofstream outputFile(out_filename, std::ios::app);

    // for (int i = 0; i < arr_rate.size(); i++) {
    //     std::vector<double> l;
    //     for (auto x : p) {
    //         l.push_back(x * arr_rate[i]);
    //     }
    //     std::string logfile_name = "Results/logfile-nClasses" + std::to_string(sizes.size()) + "-N" +
    //         std::to_string(n) + "-Win" + std::to_string(w) + "-" + sampling_name[sampling_method] + "-" + cell + "-"
    //         +
    //         "-lambda" + std::to_string(arr_rate[i]) + ".csv";
    // }

    std::vector<ExperimentStats> experiments_stats(1);

    std::vector<std::thread> threads(1);

    for (int i = 0; i < 1; i++) {
        threads[i] = std::thread(run_simulation, std::ref(conf), std::ref(experiments_stats[i]));
    }
    for (int i = 0; i < 1; i++) {
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

    for (int i = 0; i < 1; i++) {

        outputFile << 0.1 << ";";
        outputFile << experiments_stats[i] << "\n";
    }

    // Close the file
    outputFile.close();

    return 0;
}
