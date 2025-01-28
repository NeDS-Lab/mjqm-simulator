//
// Created by mccio on 24/01/25.
//

#ifndef LOADER_H
#define LOADER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void read_classes(std::string& filename, std::vector<double>& p, std::vector<int>& sizes, std::vector<double>& mus) {
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

void from_argv(char** argv, std::vector<double>& p, std::vector<int>& sizes, std::vector<double>& mus,
               std::vector<double>& arr_rate, std::vector<std::string>& headers, std::string& cell, int& n, int& w,
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

    headers = {"Arrival Rate"};

    std::string classes_filename = "Inputs/" + cell + ".txt";
    std::cout << classes_filename << std::endl;
    read_classes(classes_filename, p, sizes, mus);
    std::string lambdas_filename = "Inputs/arrRate_" + cell + ".txt";
    std::cout << lambdas_filename << std::endl;
    read_lambdas(lambdas_filename, arr_rate);

    out_filename = "Results/overLambdas-nClasses" + std::to_string(sizes.size()) + "-N" + std::to_string(n) + "-Win" +
        std::to_string(w) + "-" + sampling_name[sampling_method] + "-" + cell + ".csv";
}

#endif // LOADER_H
