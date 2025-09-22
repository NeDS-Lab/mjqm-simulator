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

std::unique_ptr<Policy> kill_smart_builder(const toml::table& data, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    const auto max_kill_cycle = data.at_path("policy.k").value<int>().value_or(10);
    const auto kill_threshold = data.at_path("policy.v").value<int>().value_or(1);
    //if (kill_threshold > max_stopped_size) {
    //    std::cerr << "v cannot be higher than k" << std::endl;
    //    return;
    //}
    return std::make_unique<KillSmart>(-16, conf.cores, n_classes, sizes, max_kill_cycle, kill_threshold);
}

std::unique_ptr<Policy> quick_swap_builder(const toml::table& data, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    const auto threshold = data.at_path("policy.threshold").value<int>().value_or(1);
    return std::make_unique<QuickSwap>(-4, conf.cores, n_classes, sizes, threshold);
}

std::unique_ptr<Policy> first_fit_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<FirstFit>(-14, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> adaptive_msf_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<AdaptiveMSF>(-7, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> static_msf_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<StaticMSF>(-8, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> most_server_first_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<MostServerFirst>(0, conf.cores, n_classes, sizes);
}
