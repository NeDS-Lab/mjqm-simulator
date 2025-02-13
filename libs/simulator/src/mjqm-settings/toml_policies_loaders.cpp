//
// Created by Marco Ciotola on 04/02/25.
//

#include <mjqm-policy/policies.h>
#include <mjqm-settings/toml_loader.h>
#include <mjqm-settings/toml_policies_loaders.h>
#include <mjqm-settings/toml_utils.h>

std::unique_ptr<Policy> smash_builder(const toml::table& data, const ExperimentConfig& conf) {
    const auto window = data.at_path("smash.window").value<unsigned int>().value_or(1);
    return std::make_unique<Smash>(window, conf.cores, conf.classes.size());
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

std::unique_ptr<Policy> most_server_first_skip_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<MostServerFirstSkip>(-4, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> most_server_first_skip_threshold_builder(const toml::table& data,
                                                                 const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    int default_threshold = static_cast<int>(
        conf.cores - sizes[0] * conf.classes[0].service_sampler->getMean() / conf.classes[0].arrival_sampler->getMean());
    int threshold = data.at_path("msf.threshold").value<int>().value_or(default_threshold);
    return std::make_unique<MostServerFirstSkipThreshold>(-5, conf.cores, n_classes, sizes, threshold);
}
