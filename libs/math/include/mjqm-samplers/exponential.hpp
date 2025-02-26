//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_EXPONENTIAL_H
#define MJQM_SAMPLERS_EXPONENTIAL_H

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include <mjqm-math/sampler.h>

class Exponential : public DistributionSampler {
public:
    explicit Exponential(const std::string_view& name, double lambda) : DistributionSampler(name), lambda(lambda) {}

    // descriptive parameters and statistics
    const double lambda;
    const double mean = 1. / lambda;
    const double variance = 1. / pow(lambda, 2);

    // operative methods
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override { return -log(randU01()) / lambda; }

    // factory methods
    static std::unique_ptr<DistributionSampler> with_rate(const std::string_view& name, const double rate) {
        return std::make_unique<Exponential>(name, rate);
    }
    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, const double mean) {
        return std::make_unique<Exponential>(name, 1. / mean);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Exponential>(name, lambda);
    }

    // string conversion
    explicit operator std::string() const override {
        return "exponential (lambda=" + std::to_string(lambda) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_EXPONENTIAL_H
