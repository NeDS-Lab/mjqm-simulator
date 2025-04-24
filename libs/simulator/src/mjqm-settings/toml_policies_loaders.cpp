//
// Created by Marco Ciotola on 04/02/25.
//

#include <mjqm-policies/policies.h>
#include <mjqm-settings/toml_loader.h>
#include <mjqm-settings/toml_policies_loaders.h>
#include <mjqm-settings/toml_utils.h>

std::unique_ptr<Policy> smash_builder(const toml::table& data, const ExperimentConfig& conf) {
    const auto window = data.at_path("policy.window").value<unsigned int>().value_or(2);
    return std::make_unique<Smash>(window, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> fifo_builder(const toml::table&, const ExperimentConfig& conf) {
    // FIFO is a special case of Smash with window 1
    return std::make_unique<Smash>(1, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> server_filling_builder(const toml::table&, const ExperimentConfig& conf) {
    return std::make_unique<ServerFilling>(-1, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> server_filling_mem_builder(const toml::table&, const ExperimentConfig& conf) {
    return std::make_unique<ServerFillingMem>(-2, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> back_filling_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<BackFilling>(-3, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> most_server_first_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<MostServerFirst>(0, conf.cores, n_classes, sizes);
}
