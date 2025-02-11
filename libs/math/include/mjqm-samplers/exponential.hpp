//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef MJQM_SAMPLERS_EXPONENTIAL_H
#define MJQM_SAMPLERS_EXPONENTIAL_H

#include <memory>
#include <mjqm-math/random.h>
#include <mjqm-math/sampler.h>
#include <random>

class exponential_rng : public rng_sampler {
public:
    explicit exponential_rng(double mean, std::shared_ptr<random_source>&& generator) :
        rng_sampler(std::move(generator)), mean(mean), lambda(1 / mean) {}

private:
    const double mean;
    const double lambda;
    const double variance = 1. / pow(lambda, 2);

public:
    double d_mean() const override { return mean; }
    double d_variance() const override { return variance; }
    double sample() override { return -log(this->rand_u01()) * mean; }

    static std::shared_ptr<sampler> with_rate(std::shared_ptr<random_source>&& generator, const double rate) {
        return std::make_shared<exponential_rng>(1. / rate, std::move(generator));
    }
    static std::shared_ptr<sampler> with_mean(std::shared_ptr<random_source>&& generator, const double mean) {
        return std::make_shared<exponential_rng>(mean, std::move(generator));
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

    static std::shared_ptr<sampler> with_rate(std::shared_ptr<std::mt19937_64> generator, const double rate) {
        return std::make_shared<exponential>(std::move(generator), 1 / rate);
    }
    static std::shared_ptr<sampler> with_mean(std::shared_ptr<std::mt19937_64> generator, const double mean) {
        return std::make_shared<exponential>(std::move(generator), mean);
    }

    explicit operator std::string() const override {
        return "exponential (lambda=" + std::to_string(lambda) + " => mean=" + std::to_string(mean) +
            " ; variance=" + std::to_string(variance) + ")";
    }
};

#endif // MJQM_SAMPLERS_EXPONENTIAL_H
