//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_BOUNDED_PARETO_H
#define MJQM_SAMPLERS_BOUNDED_PARETO_H

#include <cassert>
#include <memory>
#include <mjqm-math/random_mersenne.h>
#include <mjqm-math/sampler.h>
#include <random>

// Parameters
// L > 0 location (real)
// H > L location (real)
// α > 0 shape (real)

template <typename Generator>
class bounded_pareto_rng : public rng_sampler<Generator> {
public:
    explicit bounded_pareto_rng(std::unique_ptr<Generator>&& generator, double alpha, double l, double h) :
        rng_sampler<Generator>(std::move(generator)), l(l), h(h), alpha(alpha) {
        assert(l > 0.);
        assert(h > l);
        assert(alpha > 0.);
    }

private:
    const double l;
    const double h;
    const double alpha;
    const double mean = alpha == 1 ? h * l / (h - l) * log(h / l)
                                   : (pow(l, alpha) / (1 - pow(l / h, alpha)) * alpha / (alpha - 1) *
                                      (1 / pow(l, alpha - 1) - 1 / pow(h, alpha - 1)));
    const double variance = alpha == 2 ? 2 * pow(h, 2) * pow(l, 2) / (pow(h, 2) - pow(l, 2)) * log(h / l)
                                       : (pow(l, alpha) / (1 - pow(l / h, alpha)) * alpha / (alpha - 2) *
                                          (1 / pow(l, alpha - 2) - 1 / pow(h, alpha - 2)));

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override {
        double u = this->rand_u01();
        double num = u * pow(h, alpha) - u * pow(l, alpha) - pow(h, alpha);
        double den = pow(h, alpha) * pow(l, alpha);
        double frac = num / den;
        return pow(-frac, -1 / alpha);
    }

    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_rate(std::unique_ptr<NewGenerator>&& generator, double rate, double alpha) {
        return std::make_unique<bounded_pareto_rng<NewGenerator>>(std::move(generator), alpha, (12000.0 / 23999.0) / rate,
                                                    12000 / rate);
    }

    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_mean(std::unique_ptr<NewGenerator>&& generator, double mean, double alpha) {
        return std::make_unique<bounded_pareto_rng<NewGenerator>>(std::move(generator), alpha, (12000.0 / 23999.0) * mean,
                                                    12000 * mean);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<bounded_pareto_rng<random_mersenne>>(std::make_unique<random_mersenne>(std::move(generator), rng_sampler<Generator>::generator->name),
                                                    alpha, l, h);
    }

    explicit operator std::string() const override {
        return "bounded pareto (alpha=" + std::to_string(alpha) + " ; l=" + std::to_string(l) +
            " ; h=" + std::to_string(h) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

class bounded_pareto : public sampler {
public:
    explicit bounded_pareto(std::shared_ptr<std::mt19937_64> generator, double alpha, double l, double h) :
        generator(std::move(generator)), l(l), h(h), alpha(alpha) {
        assert(l > 0.);
        assert(h > l);
        assert(alpha > 0.);
    }

private:
    std::uniform_real_distribution<> random_uniform{0, 1};
    const std::shared_ptr<std::mt19937_64> generator;
    const double l;
    const double h;
    const double alpha;
    const double mean = alpha == 1 ? h * l / (h - l) * log(h / l)
                                   : (pow(l, alpha) / (1 - pow(l / h, alpha)) * alpha / (alpha - 1) *
                                      (1 / pow(l, alpha - 1) - 1 / pow(h, alpha - 1)));
    const double variance = alpha == 2 ? 2 * pow(h, 2) * pow(l, 2) / (pow(h, 2) - pow(l, 2)) * log(h / l)
                                       : (pow(l, alpha) / (1 - pow(l / h, alpha)) * alpha / (alpha - 2) *
                                          (1 / pow(l, alpha - 2) - 1 / pow(h, alpha - 2)));

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override {
        double u = random_uniform(*generator);
        double num = u * pow(h, alpha) - u * pow(l, alpha) - pow(h, alpha);
        double den = pow(h, alpha) * pow(l, alpha);
        double frac = num / den;
        return pow(-frac, -1 / alpha);
    }

    static std::unique_ptr<sampler> with_rate(std::shared_ptr<std::mt19937_64> generator, double rate, double alpha) {
        return std::make_unique<bounded_pareto>(std::move(generator), alpha, (12000.0 / 23999.0) / rate, 12000 / rate);
    }

    static std::unique_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, double mean, double alpha) {
        return std::make_unique<bounded_pareto>(std::move(generator), alpha, (12000.0 / 23999.0) * mean, 12000 * mean);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<bounded_pareto>(std::move(generator), alpha, l, h);
    }

    explicit operator std::string() const override {
        return "bounded pareto (alpha=" + std::to_string(alpha) + " ; l=" + std::to_string(l) +
            " ; h=" + std::to_string(h) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_BOUNDED_PARETO_H
