//
// Created by mccio on 24/01/25.
//

#ifndef PARETO_H
#define PARETO_H


#include <memory>
#include <random>
#include "exponential.hpp"


class pareto : public exponential
{
public:
    explicit pareto(std::shared_ptr<std::mt19937_64> generator, double alpha, double xm) :
        exponential(std::move(generator), 1 / alpha), xm(xm)
    {}

private:
    double xm;

public:
    double sample() override { return xm * exp(exponential::sample()); }

    static std::unique_ptr<sampler> with_rate(const std::shared_ptr<std::mt19937_64>& generator, double rate,
                                              double alpha)
    {
        return std::make_unique<pareto>(std::move(generator), alpha, (alpha - 1) / alpha / rate);
    }
    static std::unique_ptr<sampler> with_mean(const std::shared_ptr<std::mt19937_64>& generator, double mean,
                                              double alpha)
    {
        return std::make_unique<pareto>(std::move(generator), alpha, (alpha - 1) / alpha * mean);
    }
};


#endif // PARETO_H
