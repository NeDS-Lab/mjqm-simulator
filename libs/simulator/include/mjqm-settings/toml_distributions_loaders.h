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
#include <random>
#include <string_view>
#include <unordered_map>

enum distribution_use { ARRIVAL, SERVICE };
static const std::map<std::string_view, distribution_use> distribution_use_from_key = {{"arrival", ARRIVAL},
                                                                                       {"service", SERVICE}};
static const std::map<distribution_use, std::string> distribution_use_to_key = {{ARRIVAL, "arrival"},
                                                                                {SERVICE, "service"}};
inline std::ostream& operator<<(std::ostream& os, const distribution_use& use) {
    return os << distribution_use_to_key.at(use);
}

typedef bool (*distribution_loader)(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                                    std::shared_ptr<std::mt19937_64> generator,
                                    std::unique_ptr<sampler>* distribution // out
);

template <typename VAR_TYPE=double>
std::optional<VAR_TYPE> distribution_parameter(const toml::table& data, const std::string_view& cls,
                                               const distribution_use use, const std::string_view& param) {
    return either_optional<VAR_TYPE>(data.at_path(cls).at_path(distribution_use_to_key.at(use)).at_path(param),
                                     data.at_path(distribution_use_to_key.at(use)).at_path(param));
}

template <typename VAR_TYPE=double, typename... ALTS>
std::optional<VAR_TYPE> distribution_parameter(const toml::table& data, const std::string_view& cls,
                                               const distribution_use use, const std::string_view& param,
                                               const ALTS... alt_params) {
    auto opt_current = distribution_parameter<VAR_TYPE>(data, cls, use, param);
    if (opt_current.has_value()) {
        return opt_current;
    }
    return distribution_parameter<VAR_TYPE>(data, cls, use, alt_params...);
}

bool load_bounded_pareto(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                         std::shared_ptr<std::mt19937_64> generator,
                         std::unique_ptr<sampler>* distribution // out
);

bool load_deterministic(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                        std::shared_ptr<std::mt19937_64>,
                        std::unique_ptr<sampler>* distribution // out
);

bool load_exponential(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                      std::shared_ptr<std::mt19937_64> generator,
                      std::unique_ptr<sampler>* distribution // out
);

bool load_frechet(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::shared_ptr<std::mt19937_64> generator,
                  std::unique_ptr<sampler>* distribution // out
);

bool load_uniform(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::shared_ptr<std::mt19937_64> generator,
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
                       std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                       std::unique_ptr<sampler>* sampler // out
);

#endif // TOML_DISTRIBUTIONS_LOADERS_H
