//
// Created by Marco Ciotola on 26/01/25.
//

#ifndef TOML_LOADER_H
#define TOML_LOADER_H

#include <iostream>
#include <map>
#include <memory>
#include <mjqm-math/sampler.h>
#include <mjqm-policy/policy.h>
#include <mjqm-settings/toml_utils.h>
#include <mjqm-settings/toml_overrides.h>
#include <string>

struct ClassConfig {
    std::string name;
    unsigned int cores;
    std::unique_ptr<sampler> arrival_sampler;
    std::unique_ptr<sampler> service_sampler;

    friend std::ostream& operator<<(std::ostream& os, const ClassConfig& cls) {
        os << "Class: " << cls.name << std::endl;
        os << "\tCores: " << cls.cores << std::endl;
        os << "\tArrival: " << std::string(*cls.arrival_sampler) << std::endl;
        os << "\tService: " << std::string(*cls.service_sampler) << std::endl;
        return os;
    }
    template <class Archive>
    void serialize(Archive& ar, unsigned int) {
        ar & name;
        ar & cores;
        ar & arrival_sampler;
        ar & service_sampler;
    }
};

struct ExperimentConfig {
    std::string name;
    unsigned int events;
    unsigned int repetitions;
    unsigned int cores;
    std::string policy_name;
    std::unique_ptr<Policy> policy;
    std::string generator;
    std::vector<ClassConfig> classes;
    toml::table toml;

    unsigned int get_sizes(std::vector<unsigned int>&) const;

    std::string output_filename() const {
        std::string service_dist = toml.at_path("service.distribution").value<std::string>().value_or("exponential");
        service_dist[0] = std::toupper(service_dist[0]);
        return "Results/simulator_toml/overLambdas-nClasses" + std::to_string(classes.size()) + "-N" +
               std::to_string(cores) + "-Win" + std::to_string(policy->get_w()) + "-" + service_dist + "-" + name + ".csv";
    }

    friend std::ostream& operator<<(std::ostream& os, const ExperimentConfig& conf) {
        os << "Experiment: " << conf.name << std::endl;
        os << "Events: " << conf.events << std::endl;
        os << "Repetitions: " << conf.repetitions << std::endl;
        os << "Cores: " << conf.cores << std::endl;
        os << "Policy (" << conf.policy_name << "): " << std::string(*conf.policy) << std::endl;
        os << "Generator: " << conf.generator << std::endl;
        os << "Classes: " << conf.classes.size() << std::endl;
        for (const auto& cls : conf.classes) {
            os << cls;
        }
        return os;
    }
    template <class Archive>
    void serialize(Archive& ar, unsigned int) {
        ar & name;
        ar & events;
        ar & repetitions;
        ar & cores;
        ar & policy_name;
        ar & generator;
        ar & classes;
    }
};

bool from_toml(toml::table& data, ExperimentConfig& conf);
bool from_toml(std::string_view filename, ExperimentConfig& conf);
std::unique_ptr<std::vector<std::pair<bool, ExperimentConfig>>>
from_toml(const toml::table& data, const std::map<std::string, std::vector<std::string>>& overrides = {});
std::unique_ptr<std::vector<std::pair<bool, ExperimentConfig>>>
from_toml(std::string_view filename, const std::map<std::string, std::vector<std::string>>& overrides = {});

#endif // TOML_LOADER_H
