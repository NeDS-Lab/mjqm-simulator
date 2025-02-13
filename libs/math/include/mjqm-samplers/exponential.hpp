//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_EXPONENTIAL_H
#define MJQM_SAMPLERS_EXPONENTIAL_H

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "RngStream.h"
#include "mjqm-math/sampler.h"

class Exponential : public DistributionSampler {
public:
    explicit Exponential(const std::string_view& name, double mean) :
        DistributionSampler(name.data()), generator(name.data()), mean(mean), lambda(1 / mean) {}

private:
    RngStream generator;

public: // descriptive parameters and statistics
    const double mean;
    const double lambda;
    const double variance = 1. / pow(lambda, 2);

public:
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override { return -log(generator.RandU01()) * mean; }

    static std::unique_ptr<DistributionSampler> with_rate(const std::string_view& name, const double rate) {
        return std::make_unique<Exponential>(name, 1 / rate);
    }
    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, const double mean) {
        return std::make_unique<Exponential>(name, mean);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Exponential>(name, mean);
    }

    explicit operator std::string() const override {
        return "exponential (lambda=" + std::to_string(lambda) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_EXPONENTIAL_H
