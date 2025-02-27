//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_UNIFORM_H
#define MJQM_SAMPLERS_UNIFORM_H

#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include <mjqm-math/sampler.h>

class Uniform : public DistributionSampler {
public:
    explicit Uniform(const std::string_view& name, const double min, const double max) :
        DistributionSampler(name), min(min), max(max) {
        assert(min > 0);
    }

    // descriptive parameters and statistics
    const double min;
    const double max;
    const double diff = max - min;
    const double mean = (min + max) / 2.;
    const double variance = pow(max - min, 2.) / 12.;

    // operative methods
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override { return randU01() * diff + min; }

    // factory methods
    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, double mean) {
        return std::make_unique<Uniform>(name, .5 * mean, 1.5 * mean);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Uniform>(name, min, max);
    }

    // string conversion
    explicit operator std::string() const override {
        return "uniform (range [" + std::to_string(min) + ", " + std::to_string(max) +
            ") => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_UNIFORM_H
