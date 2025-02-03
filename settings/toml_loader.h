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
#include "toml++/toml.h"

using namespace std::string_literals;

enum distribution_use { ARRIVAL, SERVICE };
static const std::map<std::string_view, distribution_use> distribution_use_from_key = {{"arrival", ARRIVAL},
                                                                                       {"service", SERVICE}};
static const std::map<distribution_use, std::string_view> distribution_use_to_key = {{ARRIVAL, "arrival"},
                                                                                     {SERVICE, "service"}};
inline std::ostream& operator<<(std::ostream& os, const distribution_use& use) {
    return os << distribution_use_to_key.at(use);
}

typedef bool (*distribution_loader)(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                                    std::shared_ptr<std::mt19937_64> generator,
                                    std::unique_ptr<sampler>* distribution // out
);

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
