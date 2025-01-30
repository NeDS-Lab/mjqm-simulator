//
// Created by mccio on 24/01/25.
//

#ifndef BOUNDED_PARETO_H
#define BOUNDED_PARETO_H

#include <memory>
#include <random>
#include "../sampler.h"

// Parameters
// L > 0 location (real)
// H > L location (real)
// α > 0 shape (real)

class bounded_pareto : public sampler {
public:
    explicit bounded_pareto(std::shared_ptr<std::mt19937_64> generator, double alpha, double l, double h) :
        generator(std::move(generator)), l(l), h(h), alpha(alpha) {}

private:
    std::uniform_real_distribution<> random_uniform{0, 1};
    std::shared_ptr<std::mt19937_64> generator;
    double l;
    double h;
    double alpha;

public:
    double sample() override {
        double u = random_uniform(*generator);
        double num = u * pow(h, alpha) - u * pow(l, alpha) - pow(h, alpha);
        double den = pow(h, alpha) * pow(l, alpha);
        double frac = num / den;
        return pow(-frac, -1 / alpha);
    }

    static std::unique_ptr<sampler> with_rate(std::shared_ptr<std::mt19937_64> generator, const double rate,
                                              double alpha) {
        return std::make_unique<bounded_pareto>(std::move(generator), alpha, (12000.0 / 23999.0) / rate, 12000 / rate);
    }
    static std::unique_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, const double mean,
                                              double alpha) {
        return std::make_unique<bounded_pareto>(std::move(generator), alpha, (12000.0 / 23999.0) * mean, 12000 * mean);
    }

    explicit operator std::string() const override {
        return "bounded pareto (alpha=" + std::to_string(alpha) + " ; l=" + std::to_string(l) +
            " ; h=" + std::to_string(h) + ")";
    }
};

#endif // BOUNDED_PARETO_H
