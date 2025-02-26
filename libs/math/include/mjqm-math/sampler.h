//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLER_H
#define MJQM_SAMPLER_H

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

/// [interface]
class DistributionSampler {
public:
    const std::string name;

    explicit DistributionSampler(std::string name) : name(std::move(name)) {}
    explicit DistributionSampler(const std::string_view& name) : name(name) {}

    virtual double sample() = 0;
    virtual double getMean() const = 0;
    virtual double getVariance() const = 0;

    virtual std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const = 0;
    inline std::unique_ptr<DistributionSampler> clone() const { return clone(name); }

    virtual ~DistributionSampler() = default;
    explicit virtual operator std::string() const = 0;

    // These methods offer a compatible layer to generators from the standard library
    typedef double result_type;
    inline double operator()() { return sample(); }
    // virtual double min() const = 0;
    // virtual double max() const = 0;
};
/// [interface]

#endif // MJQM_SAMPLER_H
