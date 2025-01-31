//
// Created by mccio on 24/01/25.
//

#ifndef UNIFORM_H
#define UNIFORM_H

#include <assert.h>
#include <memory>
#include <random>
#include "../sampler.h"

class uniform : public sampler {
public:
    explicit uniform(std::shared_ptr<std::mt19937_64> generator, const double min, const double max) :
        distribution(min, max), generator(std::move(generator)) {
        assert(distribution.min() > 0);
    }

private:
    std::uniform_real_distribution<> distribution;
    const std::shared_ptr<std::mt19937_64> generator;
    double mean = (distribution.min() + distribution.max()) / 2.;
    double variance = (mean - distribution.min()) * 2.;

public:
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

#endif // UNIFORM_H
