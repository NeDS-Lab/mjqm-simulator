//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_EXPONENTIAL_H
#define MJQM_SAMPLERS_EXPONENTIAL_H

#include <memory>
#include <mjqm-math/sampler.h>
#include <mjqm-math/random_mersenne.h>
#include <random>

template <typename Generator>
class exponential_rng : public rng_sampler<Generator> {
public:
    explicit exponential_rng(double mean, std::unique_ptr<Generator>&& generator) :
        rng_sampler<Generator>(std::move(generator)), mean(mean), lambda(1 / mean) {}

private:
    const double mean;
    const double lambda;
    const double variance = 1. / pow(lambda, 2);

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return -log(this->rand_u01()) * mean; }
    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_rate(std::unique_ptr<NewGenerator>&& generator, const double rate) {
        return std::make_unique<exponential_rng>(1. / rate, std::move(generator));
    }
    template <typename NewGenerator>
    static std::unique_ptr<sampler> with_mean(std::unique_ptr<NewGenerator>&& generator, const double mean) {
        return std::make_unique<exponential_rng>(mean, std::move(generator));
    }
    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<exponential_rng<random_mersenne>>(
            mean, std::make_unique<random_mersenne>(std::move(generator), rng_sampler<Generator>::generator->name));
    }
    explicit operator std::string() const override {
        return "exponential (lambda=" + std::to_string(lambda) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

class exponential : public sampler {
public:
    explicit exponential(std::shared_ptr<std::mt19937_64> generator, double mean) :
        generator(std::move(generator)), mean(mean), lambda(1 / mean) {}

private:
    std::uniform_real_distribution<> random_uniform{0, 1};
    const std::shared_ptr<std::mt19937_64> generator;
    const double mean;
    const double lambda;
    const double variance = 1. / pow(lambda, 2);

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return -log(random_uniform(*generator)) * mean; }
    static std::unique_ptr<sampler> with_rate(std::shared_ptr<std::mt19937_64> generator, const double rate) {
        return std::make_unique<exponential>(std::move(generator), 1 / rate);
    }
    static std::unique_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, const double mean) {
        return std::make_unique<exponential>(std::move(generator), mean);
    }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64> generator) const override {
        return std::make_unique<exponential>(std::move(generator), mean);
    }

    explicit operator std::string() const override {
        return "exponential (lambda=" + std::to_string(lambda) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_EXPONENTIAL_H
