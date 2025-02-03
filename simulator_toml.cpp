//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <pthread.h>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include "math/samplers.h"
#include "settings/experiment.hpp"
#include "settings/loader.hpp"
#include "settings/toml_loader.h"
#include "simulator/simulator.h"
#include "stats/stats.h"

void run_simulation(Experiment e, unsigned long events, unsigned int repetitions,
                    ExperimentStats& stats // out
) {
    Simulator sim(e.l, e.u, e.s, e.w, e.n, e.sm, e.logf);
    sim.reset_simulation();
    sim.reset_statistics();

    sim.simulate(events, repetitions);
    sim.produce_statistics(stats);
}

void run_simulation_new(ExperimentConfig& conf,
                        ExperimentStats& stats // out
) {
    Simulator sim(conf);
    sim.reset_simulation();
    sim.reset_statistics();

    sim.simulate(conf.events, conf.repetitions);
    sim.produce_statistics(stats);
}

int main(int argc, char* argv[]) {

    std::vector<double> p;
    std::vector<unsigned int> sizes;
    std::vector<double> mus;
    std::vector<double> arr_rate;
    std::vector<std::string> headers{"Arrival Rate"};
    std::vector<double> input_utils;

    std::string cell;
    int n;
    int w;
    int sampling_method;
    std::string type;
    int n_evs;
    int n_runs;
    std::vector<std::string> sampling_name;
    std::string out_filename;

    out_filename = "Results/simulator_smash/overLambdas-nClasses" + std::to_string(2) + "-N" + std::to_string(50) +
        "-Win" + std::to_string(1) + "-Exponential-" + std::string(argv[1]) + "-toml.csv";
    // from_argv(argv, p, sizes, mus, arr_rate, cell, n, w, sampling_method, type, n_evs, n_runs, sampling_name,
    // out_filename);
    std::ofstream outputFile(out_filename, std::ios::app);
    std::vector<ExperimentConfig> ex(1);
    ExperimentConfig conf;
    if (!from_toml("Inputs/" + std::string(argv[1]) + ".toml", conf)) {
        std::cerr << "Error reading TOML file" << std::endl;
        return 1;
    }
    sizes.push_back(1);
    sizes.push_back(50);

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
        threads[i] = std::thread(run_simulation_new, std::ref(conf), std::ref(experiments_stats[i]));
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
