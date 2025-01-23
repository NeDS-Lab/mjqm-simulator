//
// Created by mccio on 17/01/25.
//
#include "samplers.h"
#include <iostream>
#include <random>

// ***** Samplers *****


// *** Deterministic ***
double Deterministic::sample_rate(const double rate) { return 1 / rate; }
double Deterministic::sample_mu(const double mu) { return mu; }


// *** Uniform ***
template <>
double Uniform<std::mt19937_64>::sample_rate(const double rate)
{
    std::uniform_real_distribution distribution(0.5 / rate, 1.5 / rate);
    return distribution(*generator);
}
template <>
double Uniform<std::mt19937_64>::sample_mu(const double mu)
{
    std::uniform_real_distribution distribution(0.5 * mu, 1.5 * mu);
    return distribution(*generator);
}


// *** Exponential ***
template <>
double Exponential<std::mt19937_64>::sample_rate(const double rate)
{
    return -log(ru(*generator)) / rate;
}
template <>
double Exponential<std::mt19937_64>::sample_mu(const double mu)
{
    return -log(ru(*generator)) * mu;
}


// *** Pareto ***
template <>
double Pareto<std::mt19937_64>::sample_mu(const double mu)
{
    auto xm = mean_ratio * mu;
    return xm * exp(Exponential::sample_rate(alfa));
}
template <>
double Pareto<std::mt19937_64>::sample_rate(const double rate)
{
    return sample_mu(1 / rate);
}


// *** Bounded Pareto ***
template <>
double BoundedPareto<std::mt19937_64>::sample_mu(const double mu)
{
    double l = (12000.0 / 23999.0) * mu;
    double h = 12000 * mu;
    double u = ru(*generator);
    double num = u * pow(h, alfa) - u * pow(l, alfa) - pow(h, alfa);
    double den = pow(h, alfa) * pow(l, alfa);
    double frac = num / den;
    return pow(-frac, -1 / alfa);
}
template <>
double BoundedPareto<std::mt19937_64>::sample_rate(const double rate)
{
    return sample_mu(1 / rate);
}


// *** Frechet ***
template <>
double Frechet<std::mt19937_64>::sample_rate(const double rate)
{
    return s_ratio / rate * pow(-log(ru(*generator)), -1 / frec_alfa);
}
template <>
double Frechet<std::mt19937_64>::sample_mu(const double mu)
{
    return s_ratio * mu * pow(-log(ru(*generator)), -1 / frec_alfa);
}



// ***** Fixed rate samplers *****


class fixed_rate_sampler
{
protected:
    double rate;

public:
    explicit fixed_rate_sampler(const double rate) { this->rate = rate; }
    virtual double sample_fixed() = 0;
    virtual ~fixed_rate_sampler() = default;
};


template <typename generator_t>
class UniformFixedRate final : public Uniform<generator_t>, fixed_rate_sampler
{
protected:
    std::uniform_real_distribution<> distribution;

public:
    explicit UniformFixedRate(generator_t* generator, const double rate) :
        Uniform<generator_t>(generator), fixed_rate_sampler(rate)
    {
        this->distribution =
            std::uniform_real_distribution(0.5 * 1 / fixed_rate_sampler::rate, 1.5 * 1 / fixed_rate_sampler::rate);
    }

public:
    double sample_fixed() override { return distribution(*sampler<generator_t>::generator); }
    ~UniformFixedRate() override = default;
};

void test() // the 3 uniform samplers should produce the same output
{
    std::cout << "deterministic rate" << std::endl;
    Deterministic deterministic{};
    for (int i = 0; i < 10; i++)
    {
        std::cout << deterministic.sample_rate(5.0) << std::endl;
    }
    std::cout << "uniform rate" << std::endl;
    Uniform uniform{new std::mt19937_64(0)};
    for (int i = 0; i < 10; i++)
    {
        std::cout << uniform.sample_rate(5.0) << std::endl;
    }
    std::cout << "fixed rate" << std::endl;
    UniformFixedRate fixed_uniform{new std::mt19937_64(0), 5.0};
    for (int i = 0; i < 10; i++)
    {
        std::cout << fixed_uniform.sample_fixed() << std::endl;
    }
    std::cout << "fixed rate 2 " << std::endl;
    UniformFixedRate fixed_uniform_2{new std::mt19937_64(0), 5.0};
    for (int i = 0; i < 10; i++)
    {
        std::cout << fixed_uniform_2.sample_rate(5.0) << std::endl;
    }
}
