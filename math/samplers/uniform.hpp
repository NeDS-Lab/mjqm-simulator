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
    explicit uniform(std::shared_ptr<std::mt19937_64> generator, double mu) :
        distribution(0.5 * mu, 1.5 * mu), generator(std::move(generator))
    {}

private:
    std::uniform_real_distribution<> distribution;
    std::shared_ptr<std::mt19937_64> generator;

public:
    double sample() override { return distribution(*generator); }
    static std::unique_ptr<sampler> with_rate(const std::shared_ptr<std::mt19937_64>& generator, double rate)
    {
        return with_mean(generator, 1 / rate);
    }
    static std::unique_ptr<sampler> with_mean(const std::shared_ptr<std::mt19937_64>& generator, double mean)
    {
        return std::make_unique<uniform>(std::move(generator), mean);
    }
};


#endif // UNIFORM_H
