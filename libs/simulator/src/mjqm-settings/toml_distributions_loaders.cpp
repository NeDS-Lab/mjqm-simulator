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
                         std::unique_ptr<DistributionSampler>* distribution // out
) {
    const auto name = full_name(cls, use);
    const auto opt_alpha = distribution_parameter(data, cls, use, "alpha");
    const auto opt_mean = distribution_parameter(data, cls, use, "mean");
    const auto opt_rate = distribution_parameter(data, cls, use, "rate");
    const auto opt_l = distribution_parameter(data, cls, use, "l", "L");
    const auto opt_h = distribution_parameter(data, cls, use, "h", "H");
    const auto opt_prob = distribution_parameter(data, cls, use, "prob");
    if (!(opt_alpha.has_value() &&
          XOR(XOR(opt_mean.has_value(), opt_rate.has_value()), opt_l.has_value() && opt_h.has_value()))) {
        print_error("Bounded pareto distribution at path "
                    << error_highlight(name)
                    << " must have alpha defined, and either mean, rate or the l/h pair");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = BoundedPareto::with_mean(name, opt_mean.value() / opt_prob.value_or(1.), alpha);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = BoundedPareto::with_rate(name, opt_rate.value() * opt_prob.value_or(1.), alpha);
        return true;
    }
    if (opt_prob.has_value()) {
        print_error("Bounded pareto distribution at path " << error_highlight(name)
                                                           << " must have rate or mean defined when prob is defined");
        return false;
    }
    *distribution = std::make_unique<BoundedPareto>(name, alpha, opt_l.value(), opt_h.value());
    return true;
}

bool load_deterministic(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                        std::unique_ptr<DistributionSampler>* distribution // out
) {
    const auto name = full_name(cls, use);
    const auto opt_value = distribution_parameter(data, cls, use, "value", "mean");
    const auto opt_rate = distribution_parameter(data, cls, use, "rate");
    const auto opt_prob = distribution_parameter(data, cls, use, "prob");

    if (!XOR(opt_value.has_value(), opt_rate.has_value())) {
        print_error("Deterministic distribution at path " << error_highlight(name)
                                                          << " must have exactly one of value or mean defined");
        return false;
    }
    double value;
    if (opt_value.has_value()) {
        value = opt_value.value();
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
    *distribution = Deterministic::with_value(name, value);
    return true;
}

bool load_exponential(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                      std::unique_ptr<DistributionSampler>* distribution // out
) {
    const auto name = full_name(cls, use);
    const auto opt_mean = distribution_parameter(data, cls, use, "mean");
    const auto opt_lambda = distribution_parameter(data, cls, use, "lambda", "rate");
    const auto opt_prob = distribution_parameter(data, cls, use, "prob");
    if (!XOR(opt_mean.has_value(), opt_lambda.has_value())) {
        print_error("Exponential distribution at path " << error_highlight(name)
                                                        << " must have exactly one of mean or lambda/rate defined");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = Exponential::with_mean(name, opt_mean.value() / opt_prob.value_or(1.));
        return true;
    }
    *distribution = Exponential::with_rate(name, opt_lambda.value() * opt_prob.value_or(1.));
    return true;
}

bool load_frechet(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::unique_ptr<DistributionSampler>* distribution // out
) {
    const auto name = full_name(cls, use);
    const auto opt_alpha = distribution_parameter(data, cls, use, "alpha");
    const auto opt_mean = distribution_parameter(data, cls, use, "mean");
    const auto opt_rate = distribution_parameter(data, cls, use, "rate");
    const auto opt_s = distribution_parameter(data, cls, use, "s");
    const auto m = distribution_parameter(data, cls, use, "m").value_or(0.);
    const auto opt_prob = distribution_parameter(data, cls, use, "prob");
    if (!(opt_alpha.has_value() && XOR(XOR(opt_mean.has_value(), opt_s.has_value()), opt_rate.has_value()))) {
        print_error("Frechet distribution at path "
                    << error_highlight(name)
                    << " must have alpha defined, and either mean, rate or s, while m has default value 0");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = Frechet::with_mean(name, opt_mean.value() / opt_prob.value_or(1.), alpha, m);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = Frechet::with_rate(name, opt_rate.value() * opt_prob.value_or(1.), alpha, m);
        return true;
    }
    if (opt_prob.has_value()) {
        print_error("Frechet distribution at path " << error_highlight(cls << "." << use)
                                                    << " must have rate or mean defined when prob is defined");
        return false;
    }
    *distribution = std::make_unique<Frechet>(name, alpha, opt_s.value(), m, true);
    return true;
}

bool load_uniform(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::unique_ptr<DistributionSampler>* distribution // out
) {
    const auto name = full_name(cls, use);
    const auto opt_mean = distribution_parameter(data, cls, use, "mean");
    const auto opt_min = distribution_parameter(data, cls, use, "min", "a");
    const auto opt_max = distribution_parameter(data, cls, use, "max", "b");
    const auto opt_rate = distribution_parameter(data, cls, use, "rate");
    const auto opt_prob = distribution_parameter(data, cls, use, "prob");
    if (!XOR(XOR(opt_mean.has_value(), opt_rate.has_value()), opt_min.has_value() && opt_max.has_value())) {
        print_error("Uniform distribution at path "
                    << error_highlight(name)
                    << " must have either the pair of a/min and b/max defined, or mean defined");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = Uniform::with_mean(name, opt_mean.value() / opt_prob.value_or(1.));
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = Uniform::with_mean(name, 1. / (opt_rate.value() * opt_prob.value_or(1.)));
        return true;
    }
    *distribution = std::make_unique<Uniform>(name, opt_min.value(), opt_max.value());
    return true;
}

bool load_distribution(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                       std::unique_ptr<DistributionSampler>* sampler // out
) {
    auto opt_type = distribution_parameter<std::string>(data, cls, use, "distribution");
    if (!opt_type.has_value()) {
        print_error("Distribution type missing at path " << error_highlight(cls << "." << use));
        return false;
    }
    const auto& distribution = opt_type.value();
    if (!distribution_loaders.contains(distribution)) {
        print_error("Unsupported distribution " << error_highlight(distribution) << " at path "
                                                << error_highlight(cls << "." << use));
        return false;
    }
    return distribution_loaders.at(distribution)(data, cls, use, sampler);
}
