//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_DETERMINISTIC_H
#define MJQM_SAMPLERS_DETERMINISTIC_H

#include <memory>
#include <sstream>
#include <string>

#include <mjqm-samplers/sampler.h>

class Deterministic : public DistributionSampler {
public:
    // descriptive parameters and statistics
    const double value;
    const double variance = 0;

    // operative methods
    inline double get_mean() const override { return value; }
    inline double get_variance() const override { return variance; }
    inline double sample() override { return value; }

    // direct and indirect constructors
    explicit Deterministic(const std::string& name, const double value) : DistributionSampler(name), value(value) {}

    static std::unique_ptr<DistributionSampler> with_value(const std::string& name, double value) {
        return std::make_unique<Deterministic>(name, value);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string& name) const override {
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
