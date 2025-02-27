//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_DETERMINISTIC_H
#define MJQM_SAMPLERS_DETERMINISTIC_H

#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include <mjqm-math/sampler.h>

class Deterministic : public DistributionSampler {
public:
    // descriptive parameters and statistics
    const double value;
    const double variance = 0;

    // operative methods
    inline double getMean() const override { return value; }
    inline double getVariance() const override { return variance; }
    inline double sample() override { return value; }

    // direct and indirect constructors
    explicit Deterministic(const std::string_view& name, const double value) :
        DistributionSampler(name), value(value) {}

    static std::unique_ptr<DistributionSampler> with_value(const std::string_view& name, double value) {
        return std::make_unique<Deterministic>(name, value);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Deterministic>(name.data(), value);
    }

    // string conversion
    explicit operator std::string() const override {
        std::ostringstream oss;
        oss << "deterministic (mean=" << value << " => variance=0)";
        return oss.str();
    }
};

#endif // MJQM_SAMPLERS_DETERMINISTIC_H
