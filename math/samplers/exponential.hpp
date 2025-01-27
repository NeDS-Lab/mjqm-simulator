//
// Created by mccio on 24/01/25.
//

#ifndef EXPONENTIAL_H
#define EXPONENTIAL_H


#include <memory>
#include <random>
#include "../sampler.h"


class exponential : public sampler
{
public:
    explicit exponential(std::shared_ptr<std::mt19937_64> generator, double mean) :
        generator(std::move(generator)), mean(mean), lambda(1 / mean)
    {}

private:
    std::uniform_real_distribution<> random_uniform{0, 1};
    std::shared_ptr<std::mt19937_64> generator;
    double mean;
    double lambda;
    double variance = pow(mean, 2);

public:
    double sample() override { return -log(random_uniform(*generator)) * mean; }
    static std::unique_ptr<sampler> with_rate(const std::shared_ptr<std::mt19937_64> generator, double rate)
    {
        return std::make_unique<exponential>(generator, 1 / rate);
    }
    static std::unique_ptr<sampler> with_mean(const std::shared_ptr<std::mt19937_64> generator, double mean)
    {
        return std::make_unique<exponential>(generator, mean);
    }

    explicit operator std::string() const override
    {
        return "exponential (lambda=" + std::to_string(lambda) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};


#endif // EXPONENTIAL_H
