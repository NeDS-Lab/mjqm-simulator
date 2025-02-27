//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_LOGNORMAL_H
#define MJQM_SAMPLERS_LOGNORMAL_H

#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "RngStream.h"
#include "mjqm-math/sampler.h"

class Lognormal : public DistributionSampler {
public:
    explicit Lognormal(const std::string_view& name, const double mean) :
        DistributionSampler(name.data()), generator(name.data()), mean(mean) {
            std::pair<double, double> res = compute_normal_params(mean);
            this->mu = res.first;
            this->sigma = res.second;
        }

private:
    RngStream generator;

    // Compute normal distribution parameters from lognormal mean & assumed stddev
    std::pair<double,double> compute_normal_params(double mu_L) {
        double sigma_L = 0.5 * mu_L;  // Assume stddev is 50% of the mean
        double sigma = std::sqrt(std::log(1 + (sigma_L * sigma_L) / (mu_L * mu_L)));
        double mu = std::log(mu_L) - (sigma * sigma) / 2.0;
        std::pair<double, double> res(mu,sigma);
        return res;
    }

public: // descriptive parameters and statistics
    const double mean;
    const double variance = pow(0.5 * mean, 2); // Assume stddev is 50% of the mean
    double mu;
    double sigma;

public:
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override {
        double u,v,s;
        do {
            u = (generator.RandU01() * 2.0) - 1.0; // transform to (-1,1)
            v = (generator.RandU01() * 2.0) - 1.0;
            s = u * u + v * v;
        } while (s >= 1.0 || s == 0.0);

         // Standard normal variable
        double z = u * std::sqrt(-2.0 * std::log(s) / s);

        // Convert to lognormal
        return std::exp(mu + sigma * z);
    }

    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, double mean) {
        return std::make_unique<Lognormal>(name, mean);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Lognormal>(name, mean);
    }

    explicit operator std::string() const override {
        return "lognormal (mean=" + std::to_string(mean) + " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_LOGNORMAL_H
