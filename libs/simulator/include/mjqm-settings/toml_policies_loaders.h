//
// Created by Marco Ciotola on 04/02/25.
//

#ifndef TOML_POLICIES_LOADERS_H
#define TOML_POLICIES_LOADERS_H

#include <unordered_map>

#include <mjqm-policy/policy.h>
#include <mjqm-settings/toml_utils.h>
#include <mjqm-settings/toml_loader.h>

typedef std::unique_ptr<Policy> (*policy_builder)(const toml::table& data, const ExperimentConfig& conf);

std::unique_ptr<Policy> smash_builder(const toml::table& data, const ExperimentConfig& conf);

std::unique_ptr<Policy> server_filling_builder(const toml::table&, const ExperimentConfig& conf);

std::unique_ptr<Policy> server_filling_mem_builder(const toml::table&, const ExperimentConfig& conf);

std::unique_ptr<Policy> back_filling_builder(const toml::table&, const ExperimentConfig& conf);

std::unique_ptr<Policy> most_server_first_builder(const toml::table&, const ExperimentConfig& conf);

std::unique_ptr<Policy> most_server_first_skip_builder(const toml::table&, const ExperimentConfig& conf);

std::unique_ptr<Policy> most_server_first_skip_threshold_builder(const toml::table& data, const ExperimentConfig& conf);

inline static std::unordered_map<std::string_view, policy_builder> policy_builders = {
    {"smash", smash_builder},
    {"server filling", server_filling_builder},
    {"server filling memoryful", server_filling_mem_builder},
    {"back filling", back_filling_builder},
    {"most server first", most_server_first_builder},
    {"most server first skip", most_server_first_skip_builder},
    {"most server first skip threshold", most_server_first_skip_threshold_builder},
};

#endif //TOML_POLICIES_LOADERS_H
