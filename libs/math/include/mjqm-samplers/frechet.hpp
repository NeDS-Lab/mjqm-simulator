//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_FRECHET_H
#define MJQM_SAMPLERS_FRECHET_H

#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "RngStream.h"
#include "mjqm-math/sampler.h"

// Parameters
//  α ∈ ( 0 , ∞ ) shape.
//  s ∈ ( 0 , ∞ ) scale (default: s = 1)
//  m ∈ ( − ∞ , ∞ ) location of minimum (default: m = 0)
// Mean
//  mu = m + s * t_gamma(1 - 1 / alpha), for alpha > 1
//   => s = (mu - m) / t_gamma(1 - 1 / alpha)
// Variance
//  sigma^2 = s^2 * (t_gamma(1 - 2 / alpha) - t_gamma(1 - 1 / alpha)^2), for alpha > 2

class Frechet : public DistributionSampler {
public:
    explicit Frechet(const std::string_view& name, const double alpha, const double s = 1., const double m = 0.,
                     bool = true) : DistributionSampler(name), alpha(alpha), s(s), m(m) {
        assert(alpha > 1); // alpha must be greater than 1 for the mean to be finite
    }
    explicit Frechet(const std::string_view& name, const double s_ratio, const double alpha, const double rate,
                     const double m = 0.) : DistributionSampler(name), alpha(alpha), s(s_ratio / rate), m(m) {
        assert(alpha > 1); // alpha must be greater than 1 for the mean to be finite
    }

    // descriptive parameters and statistics
    const double alpha;
    const double s;
    const double m;
    const double mean = alpha > 1 ? m + (s * tgamma(1 - 1 / alpha)) : std::numeric_limits<double>::infinity();
    const double variance = alpha > 2 ? pow(s, 2) * (tgamma(1 - 2 / alpha) - pow(tgamma(1 - 1 / alpha), 2))
                                      : std::numeric_limits<double>::infinity();

private:
    const double exponent = -1 / alpha;

public:
    // operative methods
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override { return s * pow(-log(randU01()), exponent); }

    // factory methods
    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, double mean, double alpha,
                                                          double m = 0.) {
        return std::make_unique<Frechet>(name, alpha, mean / tgamma(1 - 1 / alpha), m, true);
    }

    // frechet::with_rate emulates the double division for u[i] in the original code (1/(1/u[i]))
    static std::unique_ptr<DistributionSampler> with_rate(const std::string_view& name, double rate, double alpha,
                                                          double m = 0.) {
        return std::make_unique<Frechet>(name, 1 / tgammaf(1 - 1 / alpha), alpha, rate, m);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Frechet>(name, alpha, s, m, true);
    }

    // string conversion
    explicit operator std::string() const override {
        return "frechet (alpha=" + std::to_string(alpha) + " ; s=" + std::to_string(s) + " ; m=" + std::to_string(m) +
            " => mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_FRECHET_H
