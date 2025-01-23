//
// Created by mccio on 23/01/25.
//

#ifndef SAMPLERS_H
#define SAMPLERS_H
#include <random>

class sampler_intf
{
public:
    virtual double sample_rate(double rate) = 0;
    virtual double sample_mu(double mu) = 0;
    virtual ~sampler_intf() = default;
};

template <typename generator_t>
class sampler
{
protected:
    generator_t* generator;

public:
    explicit sampler(generator_t* generator){
        this->generator = generator;
    }
    virtual double sample_rate(double rate) = 0;
    virtual double sample_mu(double mu) = 0;
    virtual ~sampler() = default; // generator is deleted in Simulator
};

class Deterministic : public sampler<std::mt19937_64>
{
public:
    explicit Deterministic() : sampler(nullptr) {}
    double sample_rate(double rate) override;
    double sample_mu(double mu) override;
};


template <typename generator_t>
class Uniform : public sampler<generator_t>
{
public:
    explicit Uniform(generator_t* generator) : sampler<generator_t>(generator) {}
    double sample_rate(double rate) override;
    double sample_mu(double mu) override;
};



template <typename generator_t>
class Exponential : public sampler<generator_t>
{
public:
    explicit Exponential(generator_t* generator) : sampler<generator_t>(generator), ru(0.0, 1.0) {}

private:
    std::uniform_real_distribution<double> ru;

public:
    double sample_rate(double rate) override;
    double sample_mu(double mu) override;
};



template <typename generator_t>
class Pareto : public Exponential<generator_t>
{
public:
    explicit Pareto(generator_t* generator) : Exponential<generator_t>(generator) {}

private:
    double alfa = 2;
    double mean_ratio = (alfa - 1) / alfa;

public:
    double sample_rate(double rate) override;
    double sample_mu(double mu) override;
};


template <typename generator_t>
class BoundedPareto : public sampler<generator_t>
{
public:
    explicit BoundedPareto(generator_t* generator) : sampler<generator_t>(generator), ru(0.0, 1.0) {}

private:
    double alfa = 2;
    std::uniform_real_distribution<double> ru;

public:
    double sample_rate(double rate) override;
    double sample_mu(double mu) override;
};


template <typename generator_t>
class Frechet : public sampler<generator_t>
{
public:
    explicit Frechet(generator_t* generator) : sampler<generator_t>(generator), ru(0.0, 1.0) {}

private:
    double frec_alfa = 2.15;
    double s_ratio = 1 / std::tgammaf(1 - 1 / frec_alfa);
    std::uniform_real_distribution<double> ru;

public:
    double sample_rate(double rate) override;
    double sample_mu(double mu) override;
};

#endif // SAMPLERS_H
