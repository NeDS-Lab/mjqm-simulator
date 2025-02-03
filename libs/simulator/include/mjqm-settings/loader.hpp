//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef LOADER_H
#define LOADER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mjqm-simulator/simulator.h>
#include <mjqm-policy/policies.h>

void read_classes(std::string& filename, std::vector<double>& p, std::vector<unsigned int>& sizes, std::vector<double>& mus) {
    std::vector<std::vector<std::string>> content;
    std::vector<std::string> row;
    std::string line, word;
    std::fstream file(filename, std::ios::in);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    while (getline(file, line)) {
        row.clear();

        std::stringstream str(line);

        while (getline(str, word, ','))
            row.push_back(word);
        content.push_back(row);
    }

    for (int i = 0; i < content.size(); i++) {
        content[i][0].erase(content[i][0].begin());
        content[i][2].pop_back();

        sizes.push_back(std::stoi(content[i][0]));
        p.push_back(std::stod(content[i][1]));
        mus.push_back(std::stod(content[i][2]));
    }

    double sum = 0.0;
    for (auto x : p)
        sum += x;

    for (auto& x : p)
        x /= sum;
}

void read_lambdas(const std::string& filename, std::vector<double>& values) {
    // Open the file
    std::ifstream file(filename, std::ios::in);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    std::string line;
    std::string content;

    // Read the file line by line
    while (std::getline(file, line)) {
        content += line;
    }

    // Close the file
    file.close();

    // Find the opening and closing square brackets
    size_t start = content.find('[');
    size_t end = content.find(']');

    if (start != std::string::npos && end != std::string::npos) {
        // Extract the content within square brackets
        content = content.substr(start + 1, end - start - 1);

        // Parse the content to extract double values
        std::istringstream iss(content);
        std::string token;
        while (std::getline(iss, token, ',')) {
            values.push_back(std::stod(token)); // Convert the token to a double
        }
    } else {
        std::cerr << "No valid double values enclosed in square brackets found in the file." << std::endl;
    }
}

void from_argv(char** argv, std::vector<double>& p, std::vector<unsigned int>& sizes, std::vector<double>& mus,
               std::vector<double>& arr_rate, std::string& cell, int& n, int& w,
               int& sampling_method, std::string& type, int& n_evs, int& n_runs,
               std::vector<std::string>& sampling_name, std::string& out_filename) {
    cell = std::string(argv[1]);
    n = std::stoi(argv[2]);
    w = std::stoi(argv[3]);
    std::unordered_map<std::string, int> sampling_input = {{"exp", 0}, {"par", 1},  {"det", 2},
                                                           {"uni", 3}, {"bpar", 4}, {"fre", 5}};
    sampling_method = sampling_input[argv[4]]; // 0->exp, 1->par, 2->det, 3->uni, 4->bpar
    n_evs = std::stoi(argv[5]);
    n_runs = std::stoi(argv[6]);

    sampling_name = {"Exponential", "Pareto", "Deterministic", "Uniform", "BoundedPareto", "Frechet"};

    std::cout << "*** Processing - ID: " << argv[1] << " - N: " << std::to_string(n)
              << " - Policy: " << std::to_string(w) << " - Sampling: " << sampling_name[sampling_method] << " ***"
              << std::endl;

    std::string classes_filename = "Inputs/" + cell + ".txt";
    std::cout << classes_filename << std::endl;
    read_classes(classes_filename, p, sizes, mus);
    std::string lambdas_filename = "Inputs/arrRate_" + cell + ".txt";
    std::cout << lambdas_filename << std::endl;
    read_lambdas(lambdas_filename, arr_rate);

    out_filename = "Results/simulator_smash/overLambdas-nClasses" + std::to_string(sizes.size()) + "-N" + std::to_string(n) + "-Win" +
        std::to_string(w) + "-" + sampling_name[sampling_method] + "-" + cell + ".csv";
}

Simulator::Simulator(const std::vector<double>& l, const std::vector<double>& u, const std::vector<unsigned int>& sizes,
                     int w, int servers, int sampling_method, std::string logfile_name) {
    this->l = l;
    this->u = u;
    this->n = servers;
    this->sizes = sizes;
    this->w = w;
    this->sampling_method = sampling_method;
    this->rep_free_servers_distro = std::vector<double>(servers + 1);
    this->nclasses = static_cast<int>(sizes.size());
    this->fel.resize(sizes.size() * 2);
    this->job_fel.resize(sizes.size() * 2);
    this->jobs_inservice.resize(sizes.size());
    this->jobs_preempted.resize(sizes.size());
    this->curr_job_seq.resize(sizes.size());
    this->tot_job_seq.resize(sizes.size());
    this->curr_job_seq_start.resize(sizes.size());
    this->tot_job_seq_dur.resize(sizes.size());
    this->job_seq_amount.resize(sizes.size());
    this->debugMode = false;
    this->logfile_name = std::move(logfile_name);

    if (w == 0) {
        this->policy = std::make_unique<MostServerFirst>(w, servers, nclasses, sizes);
    } else if (w == -1) {
        this->policy = std::make_unique<ServerFilling>(w, servers, nclasses);
    } else if (w == -2) {
        this->policy = std::make_unique<ServerFillingMem>(w, servers, nclasses);
    } else if (w == -3) {
        this->policy = std::make_unique<BackFilling>(w, servers, nclasses, sizes);
    } else if (w == -4) {
        this->policy = std::make_unique<MostServerFirstSkip>(w, servers, nclasses, sizes);
    } else if (w == -5) {
        this->policy = std::make_unique<MostServerFirstSkipThreshold>(
            w, servers, nclasses, sizes, servers - static_cast<int>(sizes[0] * (l[0] / (1 / u[0]))));
    } else {
        this->policy = std::make_unique<Smash>(w, servers, nclasses);
    }

    occupancy_buf.resize(sizes.size());
    occupancy_ser.resize(sizes.size());
    completion.resize(sizes.size());
    preemption.resize(sizes.size());
    throughput.resize(sizes.size());
    waitingTime.resize(sizes.size());
    waitingTimeVar.resize(sizes.size());
    rawWaitingTime.resize(sizes.size());
    rawResponseTime.resize(sizes.size());
    responseTime.resize(sizes.size());
    responseTimeVar.resize(sizes.size());
    waste = 0;
    viol = 0;
    util = 0;
    occ = 0;
    std::uint64_t seed = 1862248485;
    generator = std::make_shared<std::mt19937_64>(next(seed));

    switch (sampling_method) {
    case 0: // exponential
        for (int i = 0; i < nclasses; i++) {
            ser_time_samplers.push_back(exponential::with_mean(generator, u[i]));
            // exponential::with_rate emulates the double division for u[i] in the original code (1/(1/u[i]))
            // ser_time_samplers.push_back(exponential::with_rate(generator, 1/u[i]));
        }
        break;
    case 1: // pareto
        for (int i = 0; i < nclasses; i++) {
            ser_time_samplers.push_back(pareto::with_mean(generator, u[i], 2));
        }
        break;
    case 2: // deterministic
        for (int i = 0; i < nclasses; i++) {
            ser_time_samplers.push_back(deterministic::with_value(u[i]));
        }
        break;
    case 4: // bounded pareto
        for (int i = 0; i < nclasses; i++) {
            ser_time_samplers.push_back(bounded_pareto::with_mean(generator, u[i], 2));
        }
        break;
    case 5: // frechet
        for (int i = 0; i < nclasses; i++) {
            // ser_time_samplers.push_back(frechet::with_mean(generator, u[i], 2.15));
            // frechet::with_rate emulates the double division for u[i] in the original code (1/(1/u[i]))
            ser_time_samplers.push_back(frechet::with_rate(generator, 1 / u[i], 2.15));
        }
        break;
    case 3: // uniform
    default:
        for (int i = 0; i < nclasses; i++) {
            ser_time_samplers.push_back(uniform::with_mean(generator, u[i]));
        }
        break;
    }

    for (int i = 0; i < nclasses; i++) {
        arr_time_samplers.push_back(exponential::with_rate(generator, l[i]));
    }
}

#endif // LOADER_H
