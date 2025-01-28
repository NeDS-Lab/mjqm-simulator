//
// Created by mccio on 24/01/25.
//

#ifndef FRECHET_H
#define FRECHET_H

#include <cassert>
#include <memory>
#include <random>
#include "../sampler.h"

// Parameters
//  α ∈ ( 0 , ∞ ) shape.
//  s ∈ ( 0 , ∞ ) scale (default: s = 1)
//  m ∈ ( − ∞ , ∞ ) location of minimum (default: m = 0)
// Mean
//  mu = m + s * t_gamma(1 - 1 / alpha), for alpha > 1
//   => s = (mu - m) / t_gamma(1 - 1 / alpha)
// Variance
//  sigma^2 = s^2 * (t_gamma(1 - 2 / alpha) - t_gamma(1 - 1 / alpha)^2), for alpha > 2

class frechet : public sampler {
public:
    explicit frechet(std::shared_ptr<std::mt19937_64> generator, const double alpha, const double s = 1.,
                     const double m = 0., bool = true) : alpha(alpha), s(s), m(m), generator(std::move(generator)) {
        assert(alpha > 1); // alpha must be greater than 1 for the mean to be finite
    }
    explicit frechet(std::shared_ptr<std::mt19937_64> generator, const double s_ratio, const double alpha,
                     const double rate, const double m = 0.) :
        alpha(alpha), s(s_ratio / rate), m(m), generator(std::move(generator)) {
        assert(alpha > 1); // alpha must be greater than 1 for the mean to be finite
    }

    const double alpha;
    const double s;
    const double m;
    const double mean = alpha > 1 ? m + (s * std::tgamma(1 - 1 / alpha)) : std::numeric_limits<double>::infinity();
    const double variance = alpha > 2 ? pow(s, 2) * (std::tgamma(1 - 2 / alpha) - pow(std::tgamma(1 - 1 / alpha), 2))
                                      : std::numeric_limits<double>::infinity();

private:
    std::uniform_real_distribution<> random_uniform{0, 1};
    std::shared_ptr<std::mt19937_64> generator;
    const double exponent = -1 / alpha;

public:
    double sample() override { return s * pow(-log(random_uniform(*generator)), exponent); }

    static std::unique_ptr<sampler> with_mean(const std::shared_ptr<std::mt19937_64> generator, double mean,
                                              double alpha, double m = 0.) {
        return std::make_unique<frechet>(std::move(generator), alpha, mean / std::tgamma(1 - 1 / alpha), m, true);
    }

    // frechet::with_rate emulates the double division for u[i] in the original code (1/(1/u[i]))
    static std::unique_ptr<sampler> with_rate(const std::shared_ptr<std::mt19937_64> generator, double rate,
                                              double alpha, double m = 0.) {
        return std::make_unique<frechet>(std::move(generator), 1 / std::tgammaf(1 - 1 / alpha), alpha, rate, m);
    }

    explicit operator std::string() const override {
        return "frechet (alpha=" + std::to_string(alpha) + " ; s=" + std::to_string(s) + " ; m=" + std::to_string(m) +
            " => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // FRECHET_H
