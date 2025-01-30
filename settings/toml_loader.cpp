//
// Created by mccio on 30/01/25.
//

#include <iostream>
#include "toml_loader.h"

template <typename VAR_TYPE>
bool load(const toml::parse_result& data, const std::string_view path, VAR_TYPE& value)
{
    auto val = data.at_path(path).value<VAR_TYPE>();
    if (val.has_value())
        value = val.value();
    else {
        std::cerr << BOLDRED << "Error:" << RESET << RED << " Value missing in TOML file " << path << RESET
                  << std::endl;
        return false;
    }
    return val.has_value();
}

template <typename VAR_TYPE>
bool load(const toml::parse_result& data, const std::string_view path, VAR_TYPE& value, const VAR_TYPE& def) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    value = val.value_or(def);
    return true;
}

bool load_distribution(const toml::parse_result& data, const std::string& path,
                              DistributionConfig*& distribution, const std::string& def) {
    const auto type = data.at_path(path + ".distribution"s).value_or(def);
    if (type == "exponential"s) {
        distribution = new ExponentialDistributionConfig{data, path};
    } else if (type == "frechet"s) {
        distribution = new FrechetDistributionConfig{data, path};
    } else if (type == "bounded pareto"s) {
        distribution = new BoundedParetoDistributionConfig{data, path};
    } else {
        std::cerr << BOLDRED << "Error:" << RESET << RED << " Unsupported distribution " << BOLDRED << type << RESET
                  << RED << " at path " << BOLDRED << path << RESET << std::endl;

        return false;
    }
    if (!distribution->is_valid()) {
        std::cerr << BOLDRED << "Error:" << RESET << RED << " Invalid definition for distribution at path " << BOLDRED
                  << path << RESET << RED << ": " << BOLDRED << distribution->to_string() << RESET << std::endl;
    }
    return distribution->is_valid();
}

bool load_class_from_toml(const toml::parse_result& data, const std::string& key, ExperimentConfig& conf) {
    const auto full_key = "class."s + key;
    ClassConfig class_conf;
    class_conf.name = key;
    const bool cores_ok = load(data, full_key + ".cores", class_conf.cores);
    const bool arrival_ok = load_distribution(data, full_key + ".arrival", class_conf.arrival_distribution,
                                              conf.default_arrival_distribution);
    const bool service_ok = load_distribution(data, full_key + ".service", class_conf.service_distribution,
                                              conf.default_service_distribution);
    if (cores_ok && arrival_ok && service_ok)
        conf.classes.push_back(class_conf);
    return cores_ok && arrival_ok && service_ok;
}

bool from_toml(const std::string_view filename, ExperimentConfig& conf) {
    const auto data = toml::parse_file(filename);
    bool ok = true;
    ok = ok && load(data, "simulation.identifier", conf.name);
    ok = ok && load(data, "simulation.events", conf.events);
    ok = ok && load(data, "simulation.repetitions", conf.repetitions);
    ok = ok && load(data, "simulation.cores", conf.cores);
    ok = ok && load(data, "simulation.policy", conf.policy, "smashed"s);

    load(data, "simulation.arrival.distribution", conf.default_arrival_distribution, "exponential"s);
    load(data, "simulation.service.distribution", conf.default_service_distribution, "exponential"s);

    const auto classes = data["class"];
    ok = ok && classes.is_table();

    if (ok)
        for (const auto& [key, value] : *classes.as_table()) {
            ok = load_class_from_toml(data, key.data(), conf) && ok; // keep going if one fails
        }

    return ok;
}
