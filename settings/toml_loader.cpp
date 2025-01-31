//
// Created by mccio on 30/01/25.
//

#include <iostream>
#include "toml_loader.h"

#include <ranges>
#include <unordered_map>

#ifndef XOR
#define XOR(a, b) (!(a) != !(b))
#endif // XOR

#ifndef error_highlight
#define error_highlight(a) BOLDRED << a << RESET << RED
#endif // error_highlight

#ifndef print_error
#define print_error(a) std::cerr << error_highlight("Error: ") << a << RESET << std::endl
#endif // print_error

template <typename VAR_TYPE>
bool load_into(const toml::parse_result& data, const std::string_view path, VAR_TYPE& value) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    if (val.has_value()) {
        value = val.value();
        return true;
    }
    print_error("Value missing in TOML file " << path);
    return false;
}

template <typename VAR_TYPE>
bool load_into(const toml::parse_result& data, const std::string_view path, VAR_TYPE& value, const VAR_TYPE& def) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    value = val.value_or(def);
    return true;
}

template <typename VAR_TYPE>
VAR_TYPE either(const std::optional<VAR_TYPE>& first, const std::optional<VAR_TYPE>& second) {
    return first.has_value() ? first.value() : second.value();
}

template <typename VAR_TYPE>
const std::optional<VAR_TYPE>& either_optional(const std::optional<VAR_TYPE>& first,
                                               const std::optional<VAR_TYPE>& second) {
    return first.has_value() ? first : second;
}

bool load_bounded_pareto(const toml::parse_result& data, std::shared_ptr<std::mt19937_64> generator,
                         const std::string& key, std::unique_ptr<sampler>* distribution) {
    const auto opt_alpha = data.at_path(key + ".alpha").value<double>();
    const auto opt_mean = data.at_path(key + ".mean").value<double>();
    const auto opt_rate = data.at_path(key + ".rate").value<double>();
    const auto opt_l = either_optional(data.at_path(key + ".l").value<double>(),
                                       data.at_path(key + ".L").value<double>());
    const auto opt_h = either_optional(data.at_path(key + ".h").value<double>(),
                                       data.at_path(key + ".H").value<double>());
    if (!(opt_alpha.has_value() &&
        XOR(
            XOR(opt_mean.has_value(), opt_rate.has_value()),
            opt_l.has_value() && opt_h.has_value()
            ))) {
        print_error(
            "Bounded pareto distribution at path " << error_highlight(key) <<
            " must have alpha defined, and either mean, rate or the l/h pair");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = bounded_pareto::with_mean(generator, opt_mean.value(), alpha);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = bounded_pareto::with_rate(generator, opt_rate.value(), alpha);
        return true;
    }
    *distribution = std::make_unique<bounded_pareto>(generator, alpha, opt_l.value(), opt_h.value());
    return true;
}

bool load_deterministic(const toml::parse_result& data,
                        const std::string& key, std::unique_ptr<sampler>* distribution) {
    const auto opt_value = data.at_path(key + ".value").value<double>();
    const auto opt_mean = data.at_path(key + ".mean").value<double>();
    if (!XOR(opt_value.has_value(), opt_mean.has_value())) {
        print_error(
            "Deterministic distribution at path " << error_highlight(key) <<
            " must have exactly one of value or mean defined");
        return false;
    }
    *distribution = deterministic::with_value(either(opt_value, opt_mean));
    return true;
}

bool load_exponential(const toml::parse_result& data, std::shared_ptr<std::mt19937_64> generator,
                      const std::string& key, std::unique_ptr<sampler>* distribution,
                      const std::optional<double> prob_modifier = std::nullopt) {
    const auto opt_mean = data.at_path(key + ".mean").value<double>();
    const auto opt_lambda = data.at_path(key + ".lambda").value<double>();
    const auto opt_rate = data.at_path(key + ".rate").value<double>();
    if (!XOR(opt_mean.has_value(), XOR(opt_lambda.has_value(), opt_rate.has_value()))) {
        print_error(
            "Exponential distribution at path " << error_highlight(key) <<
            " must have exactly one of mean or lambda/rate defined");
        return false;
    }
    if (opt_mean.has_value()) {
        double mean = prob_modifier.has_value() ? opt_mean.value() * prob_modifier.value() : opt_mean.value();
        *distribution = exponential::with_mean(generator, mean);
        return true;
    }
    double lambda = either(opt_lambda, opt_rate);
    lambda = prob_modifier.has_value() ? lambda * prob_modifier.value() : lambda;
    *distribution = exponential::with_rate(generator, lambda);
    return true;
}

bool load_frechet(const toml::parse_result& data, std::shared_ptr<std::mt19937_64> generator,
                  const std::string& key, std::unique_ptr<sampler>* distribution) {

    const auto opt_alpha = data.at_path(key + ".alpha").value<double>();
    const auto opt_mean = data.at_path(key + ".mean").value<double>();
    const auto opt_rate = data.at_path(key + ".rate").value<double>();
    const auto opt_s = data.at_path(key + ".s").value<double>();
    const double m = data.at_path(key + ".m").value<double>().value_or(0.);
    if (!(opt_alpha.has_value() &&
        XOR(XOR(opt_mean.has_value(), opt_s.has_value()), opt_rate.has_value()))) {
        print_error(
            "Frechet distribution at path " << error_highlight(key) <<
            " must have alpha defined, and either mean, rate or s, while m has default value 0");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = frechet::with_mean(generator, opt_mean.value(), alpha, m);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = frechet::with_rate(generator, opt_rate.value(), alpha, m);
        return true;
    }
    *distribution = std::make_unique<frechet>(generator, alpha, opt_s.value(), m, true);
    return true;
}

bool load_pareto(const toml::parse_result& data, std::shared_ptr<std::mt19937_64> generator,
                 const std::string& key, std::unique_ptr<sampler>* distribution) {
    const auto opt_alpha = data.at_path(key + ".alpha").value<double>();
    const auto opt_mean = data.at_path(key + ".mean").value<double>();
    const auto opt_rate = data.at_path(key + ".rate").value<double>();
    const auto opt_xm = either_optional(data.at_path(key + ".xm").value<double>(),
                                        data.at_path(key + ".Xm").value<double>());
    if (!(opt_alpha.has_value() &&
        XOR(XOR(opt_mean.has_value(), opt_rate.has_value()), opt_xm.has_value()))) {
        print_error(
            "Pareto distribution at path " << error_highlight(key) <<
            " must have alpha defined, and either mean, rate or xm");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = pareto::with_mean(generator, opt_mean.value(), alpha);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = pareto::with_rate(generator, opt_rate.value(), alpha);
        return true;
    }
    *distribution = std::make_unique<pareto>(generator, alpha, opt_xm.value());
    return true;
}

bool load_uniform(const toml::parse_result& data, std::shared_ptr<std::mt19937_64> generator,
                  const std::string& key, std::unique_ptr<sampler>* distribution) {
    const auto opt_mean = data.at_path(key + ".mean").value<double>();
    const double variance = data.at_path(key + ".variance").value<double>().value_or(1.);
    const auto opt_min = either_optional(data.at_path(key + ".a").value<double>(),
                                         data.at_path(key + ".min").value<double>());
    const auto opt_max = either_optional(data.at_path(key + ".b").value<double>(),
                                         data.at_path(key + ".max").value<double>());
    if (!XOR(opt_mean.has_value(), opt_min.has_value() && opt_max.has_value())) {
        print_error(
            "Uniform distribution at path " << error_highlight(key) <<
            " must have either the pair of a/min and b/max defined, or mean defined with optional variance (default 1)");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = uniform::with_mean(generator, opt_mean.value(), variance);
        return true;
    }
    *distribution = std::make_unique<uniform>(generator, opt_min.value(), opt_max.value());
    return true;
}


bool load_distribution(const toml::parse_result& data, const std::string& path,
                       std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                       std::unique_ptr<sampler>* sampler, const std::string& def,
                       const std::optional<double> prob_modifier) {
    const auto type = data.at_path(path + ".distribution"s).value_or(def);
    if (type == "exponential"s) {
        return load_exponential(data, generator, path, sampler, prob_modifier);
    }
    if (prob_modifier.has_value()) {
        print_error(
            "prob has been defined at path " << error_highlight(path) << " but " << error_highlight(type) <<
            " distribution doesn't support it");
        return false;
    }
    if (type == "deterministic"s) {
        return load_deterministic(data, path, sampler);
    }
    if (type == "bounded pareto"s) {
        return load_bounded_pareto(data, generator, path, sampler);
    }
    if (type == "frechet"s) {
        return load_frechet(data, generator, path, sampler);
    }
    if (type == "pareto"s) {
        return load_pareto(data, generator, path, sampler);
    }
    if (type == "uniform"s) {
        return load_uniform(data, generator, path, sampler);
    }
    print_error("Unsupported distribution " << error_highlight(type) << " at path " << error_highlight(path));
    return false;
}

bool load_class_from_toml(const toml::parse_result& data, const std::string& key, ExperimentConfig& conf,
                          std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                          const std::optional<double> arrival_modifier) {
    const auto full_key = "class."s + key;
    ClassConfig& class_conf = conf.classes[key];;
    class_conf.name = key;
    const bool cores_ok = load_into(data, full_key + ".cores", class_conf.cores);
    const bool arrival_ok = load_distribution(data, full_key + ".arrival", generator, &class_conf.arrival_sampler,
                                              conf.default_arrival_distribution,
                                              arrival_modifier);
    const bool service_ok = load_distribution(data, full_key + ".service", generator, &class_conf.service_sampler,
                                              conf.default_service_distribution);
    return cores_ok && arrival_ok && service_ok;
}

bool load_probs(const toml::impl::wrap_node<toml::table>& classes,
                std::unordered_map<std::string, double>& arrival_probs) {
    const size_t n_classes = classes.size();
    for (const auto& [key, value] : classes) {
        const auto arrival_prob = value.at_path("arrival.prob").value<double>();
        if (arrival_prob.has_value()) {
            arrival_probs[key.data()] = arrival_prob.value();
        }
    }
    if (!arrival_probs.empty()) {
        if (arrival_probs.size() == n_classes) {
            double sum = 0.0;
            for (const auto p : std::views::values(arrival_probs)) {
                sum += p;
            }
            for (auto& p : std::views::values(arrival_probs)) {
                p /= sum;
            }
        } else {
            print_error("Not all classes have the prob property defined. Define it for none or for all.");
            return false;
        }
    }
    return true;
}

bool from_toml(const std::string_view filename, ExperimentConfig& conf) {
    const auto data = toml::parse_file(filename);
    bool ok = true;
    ok = ok && load_into(data, "simulation.identifier", conf.name);
    ok = ok && load_into(data, "simulation.events", conf.events);
    ok = ok && load_into(data, "simulation.repetitions", conf.repetitions);
    ok = ok && load_into(data, "simulation.cores", conf.cores);
    ok = ok && load_into(data, "simulation.policy", conf.policy, "smashed"s);
    ok = ok && load_into(data, "simulation.generator", conf.generator, "mersenne"s);

    load_into(data, "simulation.arrival.distribution", conf.default_arrival_distribution, "exponential"s);
    load_into(data, "simulation.service.distribution", conf.default_service_distribution, "exponential"s);

    const auto class_c = data["class"];
    ok = ok && class_c.is_table();
    auto generator = std::make_shared<std::mt19937_64>();

    if (ok) {
        const auto classes = *class_c.as_table();
        std::unordered_map<std::string, double> arrival_probs;
        ok = load_probs(classes, arrival_probs) && ok;
        for (const auto& [key, value] : classes) {
            const auto arrival_modifier = arrival_probs.contains(key.data())
                ? std::optional(arrival_probs[key.data()])
                : std::nullopt;
            ok = load_class_from_toml(data, key.data(), conf, generator, arrival_modifier) && ok;
            // keep going if one soft fails
        }
    }

    return ok;
}

#undef print_error
