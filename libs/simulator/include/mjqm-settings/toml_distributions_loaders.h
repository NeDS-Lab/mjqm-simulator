//
// Created by Marco Ciotola on 04/02/25.
//

#ifndef TOML_DISTRIBUTIONS_LOADERS_H
#define TOML_DISTRIBUTIONS_LOADERS_H

#include <iostream>
#include <map>
#include <memory>
#include <mjqm-math/samplers.h>
#include <mjqm-settings/toml_utils.h>
#include <string_view>
#include <unordered_map>
#include "mjqm-math/random_mersenne.h"

enum distribution_use { ARRIVAL, SERVICE };
static const std::map<std::string_view, distribution_use> distribution_use_from_key = {{"arrival", ARRIVAL},
                                                                                       {"service", SERVICE}};
static const std::map<distribution_use, std::string> distribution_use_to_key = {{ARRIVAL, "arrival"},
                                                                                {SERVICE, "service"}};
inline std::ostream& operator<<(std::ostream& os, const distribution_use& use) {
    return os << distribution_use_to_key.at(use);
}

typedef bool (*distribution_loader)(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                                    random_source_factory<random_mersenne>& generator,
                                    std::unique_ptr<sampler>* distribution // out
);

template <typename VAR_TYPE>
std::optional<VAR_TYPE> distribution_parameter(const toml::table& data, const std::string_view& cls,
                                               const distribution_use use, const std::string_view& param) {
    return either_optional<VAR_TYPE>(data.at_path(cls).at_path(distribution_use_to_key.at(use)).at_path(param),
                                     data.at_path(distribution_use_to_key.at(use)).at_path(param));
}

bool load_bounded_pareto(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                         random_source_factory<random_mersenne>& generator,
                         std::unique_ptr<sampler>* distribution // out
);

bool load_deterministic(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                        random_source_factory<random_mersenne>&,
                        std::unique_ptr<sampler>* distribution // out
);

bool load_exponential(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                      random_source_factory<random_mersenne>& generator,
                      std::unique_ptr<sampler>* distribution // out
);

bool load_frechet(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  random_source_factory<random_mersenne>& generator,
                  std::unique_ptr<sampler>* distribution // out
);

bool load_uniform(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  random_source_factory<random_mersenne>& generator,
                  std::unique_ptr<sampler>* distribution // out
);

inline static std::unordered_map<std::string_view, distribution_loader> distribution_loaders = {
    {"exponential", load_exponential},
    {"deterministic", load_deterministic},
    {"bounded pareto", load_bounded_pareto},
    {"frechet", load_frechet},
    {"uniform", load_uniform},
};

bool load_distribution(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                       random_source_factory<random_mersenne>& generator,
                       std::unique_ptr<sampler>* sampler // out
);

#endif // TOML_DISTRIBUTIONS_LOADERS_H
