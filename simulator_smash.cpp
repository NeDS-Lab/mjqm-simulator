//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <string>
#include <thread>
#include <vector>

#include <mjqm-settings/loader.hpp>
#include <mjqm-simulator/experiment.h>
#include <mjqm-simulator/experiment_stats.h>
#include <mjqm-simulator/simulator.h>

void run_simulation(Experiment e, unsigned long events, unsigned int repetitions,
                    ExperimentStats& stats // out
) {
    Simulator sim(e.l, e.u, e.s, e.w, e.n, e.sm, e.logf, stats);
    sim.reset_simulation();
    sim.reset_statistics();

    sim.simulate(events, repetitions);
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
    from_argv(argv, p, sizes, mus, arr_rate, cell, n, w, sampling_method, type, n_evs, n_runs, sampling_name,
              out_filename);
    std::ofstream outputFile(out_filename, std::ios::app);
    std::vector<Experiment> ex;

    for (size_t i = 0; i < arr_rate.size(); i++) {
        std::vector<double> l;
        for (auto x : p) {
            l.push_back(x * arr_rate[i]);
        }
        std::string logfile_name = "Results/logfile-nClasses" + std::to_string(sizes.size()) + "-N" +
            std::to_string(n) + "-Win" + std::to_string(w) + "-" + sampling_name[sampling_method] + "-" + cell + "-" +
            "-lambda" + std::to_string(arr_rate[i]) + ".csv";
        ex.push_back(Experiment{l, mus, sizes, w, n, sampling_method, logfile_name});
    }

    std::vector<ExperimentStats> experiments_stats(ex.size());

    std::vector<std::thread> threads(ex.size());

    for (size_t i = 0; i < ex.size(); i++) {
        threads[i] = std::thread(run_simulation, ex[i], n_evs, n_runs, std::ref(experiments_stats[i]));
    }
    for (size_t i = 0; i < ex.size(); i++) {
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

    for (size_t i = 0; i < ex.size(); i++) {

        outputFile << arr_rate[i] << ";";
        outputFile << experiments_stats[i] << "\n";
    }

    // Close the file
    outputFile.close();

    return 0;
}
