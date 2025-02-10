//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_FRECHET_H
#define MJQM_SAMPLERS_FRECHET_H

#include <cassert>
#include <memory>
#include <mjqm-math/sampler.h>
#include <random>
#include "mjqm-math/random_mersenne.h"

// Parameters
//  α ∈ ( 0 , ∞ ) shape.
//  s ∈ ( 0 , ∞ ) scale (default: s = 1)
//  m ∈ ( − ∞ , ∞ ) location of minimum (default: m = 0)
// Mean
//  mu = m + s * t_gamma(1 - 1 / alpha), for alpha > 1
//   => s = (mu - m) / t_gamma(1 - 1 / alpha)
// Variance
//  sigma^2 = s^2 * (t_gamma(1 - 2 / alpha) - t_gamma(1 - 1 / alpha)^2), for alpha > 2

template <typename Generator>
class frechet_rng : public rng_sampler<Generator> {
public:
    explicit frechet_rng(std::unique_ptr<Generator>&& generator, const double alpha, const double s = 1.,
                     const double m = 0., bool = true) : rng_sampler<Generator>(std::move(generator)), alpha(alpha), s(s), m(m) {
        assert(alpha > 1); // alpha must be greater than 1 for the mean to be finite
    }
    explicit frechet_rng(std::unique_ptr<Generator>&& generator, const double s_ratio, const double alpha,
                     const double rate, const double m = 0.) :rng_sampler<Generator>(std::move(generator)),
        alpha(alpha), s(s_ratio / rate), m(m) {
        assert(alpha > 1); // alpha must be greater than 1 for the mean to be finite
    }

    const double alpha;
    const double s;
    const double m;
    const double mean = alpha > 1 ? m + (s * std::tgamma(1 - 1 / alpha)) : std::numeric_limits<double>::infinity();
    const double variance = alpha > 2 ? pow(s, 2) * (std::tgamma(1 - 2 / alpha) - pow(std::tgamma(1 - 1 / alpha), 2))
                                      : std::numeric_limits<double>::infinity();

private:
    const double exponent = -1 / alpha;

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return s * pow(-log(this->rand_u01()), exponent); }

    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_mean(std::unique_ptr<NewGenerator>&& generator, double mean, double alpha,
                                              double m = 0.) {
        return std::make_unique<frechet_rng>(std::move(generator), alpha, mean / std::tgamma(1 - 1 / alpha), m, true);
    }

    // frechet::with_rate emulates the double division for u[i] in the original code (1/(1/u[i]))
    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_rate(std::unique_ptr<NewGenerator>&& generator, double rate, double alpha,
                                              double m = 0.) {
        return std::make_unique<frechet_rng>(std::move(generator), 1 / std::tgammaf(1 - 1 / alpha), alpha, rate, m);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<frechet_rng<random_mersenne>>(std::make_unique<random_mersenne>(std::move(generator), rng_sampler<Generator>::generator->name), alpha, s, m, true);
    }

    explicit operator std::string() const override {
        return "frechet (alpha=" + std::to_string(alpha) + " ; s=" + std::to_string(s) + " ; m=" + std::to_string(m) +
            " => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};


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
    const std::shared_ptr<std::mt19937_64> generator;
    const double exponent = -1 / alpha;

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return s * pow(-log(random_uniform(*generator)), exponent); }

    static std::unique_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, double mean, double alpha,
                                              double m = 0.) {
        return std::make_unique<frechet>(std::move(generator), alpha, mean / std::tgamma(1 - 1 / alpha), m, true);
    }

    // frechet::with_rate emulates the double division for u[i] in the original code (1/(1/u[i]))
    static std::unique_ptr<sampler> with_rate(std::shared_ptr<std::mt19937_64> generator, double rate, double alpha,
                                              double m = 0.) {
        return std::make_unique<frechet>(std::move(generator), 1 / std::tgammaf(1 - 1 / alpha), alpha, rate, m);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<frechet>(std::move(generator), alpha, s, m, true);
    }

    explicit operator std::string() const override {
        return "frechet (alpha=" + std::to_string(alpha) + " ; s=" + std::to_string(s) + " ; m=" + std::to_string(m) +
            " => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_FRECHET_H
