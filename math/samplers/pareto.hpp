//
// Created by mccio on 24/01/25.
//

#ifndef PARETO_H
#define PARETO_H

#include <assert.h>
#include <memory>
#include <random>
#include "exponential.hpp"

class pareto : public exponential {
public:
    explicit pareto(std::shared_ptr<std::mt19937_64> generator, double alpha, double xm) :
        exponential(std::move(generator), 1 / alpha), alpha(alpha), xm(xm) {
        assert(alpha > 1); // otherwise the mean is infinite
    }

private:
    double alpha;
    double xm;
    double mean = xm * alpha / (alpha - 1);
    double variance = alpha > 2 ? std::pow(xm, 2) * alpha / (std::pow(alpha - 1, 2) * (alpha - 2))
                                : std::numeric_limits<double>::infinity();

public:
    double sample() override { return xm * exp(exponential::sample()); }

    static std::unique_ptr<sampler> with_rate(std::shared_ptr<std::mt19937_64> generator, double rate,
                                              double alpha) {
        return std::make_unique<pareto>(std::move(generator), alpha, (alpha - 1) / alpha / rate);
    }
    static std::unique_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, double mean,
                                              double alpha) {
        return std::make_unique<pareto>(std::move(generator), alpha, (alpha - 1) / alpha * mean);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<pareto>(std::move(generator), alpha, xm);
    }

    explicit operator std::string() const override {
        return "pareto (alpha=" + std::to_string(alpha) + " ; x_m=" + std::to_string(xm) +
            " => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // PARETO_H
