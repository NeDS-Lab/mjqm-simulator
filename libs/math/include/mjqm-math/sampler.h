//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLER_H
#define MJQM_SAMPLER_H

#include <memory>
#include <mjqm-math/random.h>
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

template <typename RandomSource>
class rng_sampler : public sampler {
protected:
    std::unique_ptr<RandomSource> generator;
    double rand_u01() { return generator->RandU01(); }

public:
    explicit rng_sampler(std::unique_ptr<RandomSource>&& generator) : generator(std::move(generator)) {}
    double sample() override = 0;
    double d_mean() const override = 0;
    double d_variance() const override = 0;
    explicit operator std::string() const override = 0;
    ~rng_sampler() override = default;
};

#endif // MJQM_SAMPLER_H
