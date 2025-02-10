//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_UNIFORM_H
#define MJQM_SAMPLERS_UNIFORM_H

#include <cassert>
#include <memory>
#include <mjqm-math/sampler.h>
#include <random>
#include "mjqm-math/random_mersenne.h"

template <typename Generator>
class uniform_rng : public rng_sampler<Generator> {
public:
    explicit uniform_rng(std::unique_ptr<Generator>&& generator, const double min, const double max) :
      rng_sampler<Generator>(std::move(generator)),  distribution(min, max) {
        assert(distribution.min() > 0);
    }

private:
    std::uniform_real_distribution<> distribution;
    const double mean = (distribution.min() + distribution.max()) / 2.;
    const double variance = (mean - distribution.min()) * 2.;

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return this->rand_u01() * (distribution.b() - distribution.a()) + distribution.a(); }

    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_mean(std::unique_ptr<NewGenerator>&& generator, double mean,
                                              double variance = 1.) {
        return std::make_unique<uniform_rng>(std::move(generator), (1. - variance / 2.) * mean,
                                         (1. + variance / 2.) * mean);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<uniform_rng<random_mersenne>>(std::make_unique<random_mersenne>(std::move(generator), rng_sampler<Generator>::generator->name), distribution.min(), distribution.max());
    }

    explicit operator std::string() const override {
        return "uniform (range [" + std::to_string(distribution.min()) + ", " + std::to_string(distribution.max()) +
            ") => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

class uniform : public sampler {
public:
    explicit uniform(std::shared_ptr<std::mt19937_64> generator, const double min, const double max) :
        distribution(min, max), generator(std::move(generator)) {
        assert(distribution.min() > 0);
    }

private:
    std::uniform_real_distribution<> distribution;
    const std::shared_ptr<std::mt19937_64> generator;
    const double mean = (distribution.min() + distribution.max()) / 2.;
    const double variance = (mean - distribution.min()) * 2.;

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return distribution(*generator); }

    static std::unique_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, double mean,
                                              double variance = 1.) {
        return std::make_unique<uniform>(std::move(generator), (1. - variance / 2.) * mean,
                                         (1. + variance / 2.) * mean);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<uniform>(std::move(generator), distribution.min(), distribution.max());
    }

    explicit operator std::string() const override {
        return "uniform (range [" + std::to_string(distribution.min()) + ", " + std::to_string(distribution.max()) +
            ") => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_UNIFORM_H
