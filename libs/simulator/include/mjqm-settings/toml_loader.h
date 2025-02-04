//
// Created by Marco Ciotola on 26/01/25.
//

#ifndef TOML_LOADER_H
#define TOML_LOADER_H

#include <mjqm-math/sampler.h>
#include <mjqm-policy/policy.h>
#include <mjqm-settings/toml_utils.h>

using namespace std::string_literals;

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
    unsigned int get_sizes(std::vector<unsigned int>&) const;

    friend std::ostream& operator<<(std::ostream& os, const ExperimentConfig& conf) {
        os << "Experiment: " << conf.name << std::endl;
        os << "Events: " << conf.events << std::endl;
        os << "Repetitions: " << conf.repetitions << std::endl;
        os << "Cores: " << conf.cores << std::endl;
        os << "Policy: " << conf.policy_name << std::endl;
        os << "Generator: " << conf.generator << std::endl;
        os << "Classes: " << conf.classes.size() << std::endl;
        for (const auto& cls : conf.classes) {
            os << cls;
        }
        return os;
    }
};

bool from_toml(std::string_view filename, ExperimentConfig& conf);

#endif // TOML_LOADER_H
