//
// Created by mccio on 24/01/25.
//

#ifndef SAMPLER_H
#define SAMPLER_H

#include <random>
#include <memory>

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

#endif // SAMPLER_H
