//
// Created by Marco Ciotola on 04/02/25.
//

#include <mjqm-math/samplers.h>
#include <mjqm-settings/toml_distributions_loaders.h>
#include <unordered_map>

#ifndef XOR
#define XOR(a, b) (!(a) != !(b))
#endif // XOR

bool load_bounded_pareto(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                         std::shared_ptr<std::mt19937_64> generator,
                         std::unique_ptr<sampler>* distribution // out
) {
    auto opt_alpha = distribution_parameter<double>(data, cls, use, "alpha");
    auto opt_mean = distribution_parameter<double>(data, cls, use, "mean");
    auto opt_rate = distribution_parameter<double>(data, cls, use, "rate");
    auto opt_l = distribution_parameter<double>(data, cls, use, "l");
    auto opt_L = distribution_parameter<double>(data, cls, use, "L");
    opt_l = either_optional(opt_l, opt_L);
    auto opt_h = distribution_parameter<double>(data, cls, use, "h");
    auto opt_H = distribution_parameter<double>(data, cls, use, "H");
    opt_h = either_optional(opt_h, opt_H);
    const auto opt_prob = distribution_parameter<double>(data, cls, use, "prob");
    if (!(opt_alpha.has_value() &&
          XOR(XOR(opt_mean.has_value(), opt_rate.has_value()), opt_l.has_value() && opt_h.has_value()))) {
        print_error("Bounded pareto distribution at path "
                    << error_highlight(cls << "." << use)
                    << " must have alpha defined, and either mean, rate or the l/h pair");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = bounded_pareto::with_mean(generator, opt_mean.value() / opt_prob.value_or(1.), alpha);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = bounded_pareto::with_rate(generator, opt_rate.value() * opt_prob.value_or(1.), alpha);
        return true;
    }
    if (opt_prob.has_value()) {
        print_error("Bounded pareto distribution at path " << error_highlight(cls << "." << use)
                                                           << " must have rate or mean defined when prob is defined");
        return false;
    }
    *distribution = std::make_unique<bounded_pareto>(generator, alpha, opt_l.value(), opt_h.value());
    return true;
}

bool load_deterministic(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                        std::shared_ptr<std::mt19937_64>,
                        std::unique_ptr<sampler>* distribution // out
) {
    const auto opt_value = distribution_parameter<double>(data, cls, use, "value");
    const auto opt_mean = distribution_parameter<double>(data, cls, use, "mean");
    const auto opt_rate = distribution_parameter<double>(data, cls, use, "rate");
    const auto opt_prob = distribution_parameter<double>(data, cls, use, "prob");

    if (!XOR(XOR(opt_value.has_value(), opt_mean.has_value()), opt_rate.has_value())) {
        print_error("Deterministic distribution at path " << error_highlight(cls << "." << use)
                                                          << " must have exactly one of value or mean defined");
        return false;
    }
    double value;
    if (opt_mean.has_value() || opt_value.has_value()) {
        value = either(opt_value, opt_mean);
        if (opt_prob.has_value()) {
            value /= opt_prob.value();
        }
    } else {
        value = opt_rate.value();
        if (opt_prob.has_value()) {
            value *= opt_prob.value();
        }
        value = 1. / value;
    }
    *distribution = deterministic::with_value(value);
    return true;
}

bool load_exponential(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                      std::shared_ptr<std::mt19937_64> generator,
                      std::unique_ptr<sampler>* distribution // out
) {
    const auto opt_mean = distribution_parameter<double>(data, cls, use, "mean");
    const auto opt_lambda = distribution_parameter<double>(data, cls, use, "lambda");
    const auto opt_rate = distribution_parameter<double>(data, cls, use, "rate");
    const auto opt_prob = distribution_parameter<double>(data, cls, use, "prob");
    if (!XOR(opt_mean.has_value(), XOR(opt_lambda.has_value(), opt_rate.has_value()))) {
        print_error("Exponential distribution at path " << error_highlight(cls << "." << use)
                                                        << " must have exactly one of mean or lambda/rate defined");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = exponential::with_mean(generator, opt_mean.value() / opt_prob.value_or(1.));
        return true;
    }
    *distribution = exponential::with_rate(generator, either(opt_lambda, opt_rate) * opt_prob.value_or(1.));
    return true;
}

bool load_frechet(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::shared_ptr<std::mt19937_64> generator,
                  std::unique_ptr<sampler>* distribution // out
) {
    auto opt_alpha = distribution_parameter<double>(data, cls, use, "alpha");
    auto opt_mean = distribution_parameter<double>(data, cls, use, "mean");
    auto opt_rate = distribution_parameter<double>(data, cls, use, "rate");
    auto opt_s = distribution_parameter<double>(data, cls, use, "s");
    auto m = distribution_parameter<double>(data, cls, use, "m").value_or(0.);
    const auto opt_prob = distribution_parameter<double>(data, cls, use, "prob");
    if (!(opt_alpha.has_value() && XOR(XOR(opt_mean.has_value(), opt_s.has_value()), opt_rate.has_value()))) {
        print_error("Frechet distribution at path "
                    << error_highlight(cls << "." << use)
                    << " must have alpha defined, and either mean, rate or s, while m has default value 0");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = frechet::with_mean(generator, opt_mean.value() / opt_prob.value_or(1.), alpha, m);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = frechet::with_rate(generator, opt_rate.value() * opt_prob.value_or(1.), alpha, m);
        return true;
    }
    if (opt_prob.has_value()) {
        print_error("Frechet distribution at path " << error_highlight(cls << "." << use)
                                                    << " must have rate or mean defined when prob is defined");
        return false;
    }
    *distribution = std::make_unique<frechet>(generator, alpha, opt_s.value(), m, true);
    return true;
}

bool load_uniform(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::shared_ptr<std::mt19937_64> generator,
                  std::unique_ptr<sampler>* distribution // out
) {
    auto opt_mean = distribution_parameter<double>(data, cls, use, "mean");
    auto opt_variance = distribution_parameter<double>(data, cls, use, "variance");
    auto opt_a = distribution_parameter<double>(data, cls, use, "a");
    auto opt_min = distribution_parameter<double>(data, cls, use, "min");
    opt_min = either_optional(opt_min, opt_a);
    auto opt_b = distribution_parameter<double>(data, cls, use, "b");
    auto opt_max = distribution_parameter<double>(data, cls, use, "max");
    opt_max = either_optional(opt_max, opt_b);
    const auto opt_rate = distribution_parameter<double>(data, cls, use, "rate");
    const auto opt_prob = distribution_parameter<double>(data, cls, use, "prob");
    if (!XOR(XOR(opt_mean.has_value(), opt_rate.has_value()), opt_min.has_value() && opt_max.has_value() && !opt_variance.has_value())) {
        print_error("Uniform distribution at path " << error_highlight(cls << "." << use)
                                                    << " must have either the pair of a/min and b/max defined, or mean "
                                                       "defined with optional variance (default 1)");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = uniform::with_mean(generator, opt_mean.value() / opt_prob.value_or(1.), opt_variance.value_or(1.));
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = uniform::with_mean(generator, 1. / (opt_rate.value() * opt_prob.value_or(1.)), opt_variance.value_or(1.));
        return true;
    }
    *distribution = std::make_unique<uniform>(generator, opt_min.value(), opt_max.value());
    return true;
}

bool load_distribution(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                       std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                       std::unique_ptr<sampler>* sampler // out
) {
    auto opt_type = distribution_parameter<std::string>(data, cls, use, "distribution");
    if (!opt_type.has_value()) {
        print_error("Distribution type missing at path " << error_highlight(cls << "." << use));
        return false;
    }
    const auto type = opt_type.value();
    if (!distribution_loaders.contains(type)) {
        print_error("Unsupported distribution " << error_highlight(type) << " at path "
                                                << error_highlight(cls << "." << use));
        return false;
    }
    return distribution_loaders.at(type)(data, cls, use, generator, sampler);
}
