//
// Created by mccio on 24/01/25.
//

#ifndef UNIFORM_H
#define UNIFORM_H

#include <memory>
#include <random>
#include "../sampler.h"


class uniform : public sampler
{
public:
    explicit uniform(std::shared_ptr<std::mt19937_64> generator, double mean) :
        distribution(0.5 * mean, 1.5 * mean), generator(std::move(generator))
    {}

private:
    std::uniform_real_distribution<> distribution;
    std::shared_ptr<std::mt19937_64> generator;

public:
    double sample() override { return distribution(*generator); }
    static std::unique_ptr<sampler> with_mean(const std::shared_ptr<std::mt19937_64>& generator, double mean)
    {
        return std::make_unique<uniform>(std::move(generator), mean);
    }

    explicit operator std::string() const override
    {
        return "uniform (range [" + std::to_string(distribution.a()) + ", " + std::to_string(distribution.b()) +
            ") => mean=" + std::to_string((distribution.a() + distribution.b()) / 2. ) + " variance=1)";;
    }
};


#endif // UNIFORM_H
