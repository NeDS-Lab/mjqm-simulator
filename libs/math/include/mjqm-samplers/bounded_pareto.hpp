//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_BOUNDED_PARETO_H
#define MJQM_SAMPLERS_BOUNDED_PARETO_H

#include <cassert>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include <mjqm-samplers/sampler.h>

// Parameters
// L > 0 location (real)
// H > L location (real)
// α > 0 shape (real)

class BoundedPareto : public DistributionSampler {
public:
    // descriptive parameters and statistics
    const double l;
    const double h;
    const double alpha;
    const double mean = alpha == 1 ? h * l / (h - l) * log(h / l)
                                   : (pow(l, alpha) / (1 - pow(l / h, alpha)) * alpha / (alpha - 1) *
                                      (1 / pow(l, alpha - 1) - 1 / pow(h, alpha - 1)));
    const double variance = alpha == 2 ? 2 * pow(h, 2) * pow(l, 2) / (pow(h, 2) - pow(l, 2)) * log(h / l)
                                       : (pow(l, alpha) / (1 - pow(l / h, alpha)) * alpha / (alpha - 2) *
                                          (1 / pow(l, alpha - 2) - 1 / pow(h, alpha - 2)));

    // operative methods
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override {
        double u = randU01();
        double num = u * pow(h, alpha) - u * pow(l, alpha) - pow(h, alpha);
        double den = pow(h, alpha) * pow(l, alpha);
        double frac = num / den;
        return pow(-frac, -1 / alpha);
    }

    // direct and indirect constructors
    explicit BoundedPareto(const std::string_view& name, double alpha, double l, double h) :
        DistributionSampler(name), l(l), h(h), alpha(alpha) {
        assert(l > 0.);
        assert(h > l);
        assert(alpha > 0.);
    }

    static std::unique_ptr<DistributionSampler> with_rate(const std::string_view& name, double rate, double alpha) {
        return std::make_unique<BoundedPareto>(name, alpha, (12000.0 / 23999.0) / rate, 12000 / rate);
    }

    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, double mean, double alpha) {
        return std::make_unique<BoundedPareto>(name, alpha, (12000.0 / 23999.0) * mean, 12000 * mean);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<BoundedPareto>(name, alpha, l, h);
    }

    // string conversion
    explicit operator std::string() const override {
        std::ostringstream oss;
        oss << "bounded pareto (alpha=" << alpha << " ; l=" << l << " ; h=" << h << " => mean=" << mean
            << " ; variance=" << variance << ")";
        return oss.str();
    }
};

#endif // MJQM_SAMPLERS_BOUNDED_PARETO_H
