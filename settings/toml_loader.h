//
// Created by mccio on 26/01/25.
//

#ifndef TOML_LOADER_H
#define TOML_LOADER_H

#define TOML_ENABLE_UNRELEASED_FEATURES 1

#define RESET "\033[0m"
#define BLACK "\033[30m" // Black
#define RED "\033[31m" // Red
#define GREEN "\033[32m" // Green
#define YELLOW "\033[33m" // Yellow
#define BLUE "\033[34m" // Blue
#define MAGENTA "\033[35m" // Magenta
#define CYAN "\033[36m" // Cyan
#define WHITE "\033[37m" // White
#define BOLDBLACK "\033[1m\033[30m" // Bold Black
#define BOLDRED "\033[1m\033[31m" // Bold Red
#define BOLDGREEN "\033[1m\033[32m" // Bold Green
#define BOLDYELLOW "\033[1m\033[33m" // Bold Yellow
#define BOLDBLUE "\033[1m\033[34m" // Bold Blue
#define BOLDMAGENTA "\033[1m\033[35m" // Bold Magenta
#define BOLDCYAN "\033[1m\033[36m" // Bold Cyan
#define BOLDWHITE "\033[1m\033[37m" // Bold White

#include "../math/samplers.h"
#include "../policies/policy.h"
#include "toml++/toml.hpp"

using namespace std::string_literals;

struct ClassConfig {
    std::string name;
    unsigned int cores;
    std::unique_ptr<sampler> arrival_sampler;
    std::unique_ptr<sampler> service_sampler;
    ~ClassConfig() = default;
};

struct ExperimentConfig {
    std::string name;
    unsigned int events;
    unsigned int repetitions;
    unsigned int cores;
    std::string policy_name;
    std::unique_ptr<Policy> policy;
    std::string generator;
    std::string default_arrival_distribution;
    std::string default_service_distribution;
    std::map<std::string_view, ClassConfig> classes_map;
    ~ExperimentConfig() = default;
    int get_sizes(std::vector<unsigned int>&) const;
};

template <typename VAR_TYPE>
bool load_into(const toml::parse_result& data, std::string_view path, VAR_TYPE& value);

template <typename VAR_TYPE>
bool load_into(const toml::parse_result& data, std::string_view path, VAR_TYPE& value, const VAR_TYPE& def);

bool load_distribution(const toml::parse_result& data, const std::string& path,
                       std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                       std::unique_ptr<sampler>* sampler, const std::string& def,
                       std::optional<double> prob_modifier=std::nullopt);

bool load_class_from_toml(const toml::parse_result& data, const std::string& key, ExperimentConfig& conf,
                          std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                          std::optional<double> arrival_modifier);

bool from_toml(std::string_view filename, ExperimentConfig& conf);

#endif // TOML_LOADER_H
