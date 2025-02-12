//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLER_H
#define MJQM_SAMPLER_H

#include <memory>
#include <random>

class sampler {
public:
    typedef double result_type; // for mirroring how std usually does it
    virtual double sample() = 0;
    virtual ~sampler() = default;
    virtual double d_mean() const = 0;
    virtual double d_variance() const = 0;
    virtual std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const = 0;

    explicit virtual operator std::string() const = 0;
};

#endif // MJQM_SAMPLER_H
