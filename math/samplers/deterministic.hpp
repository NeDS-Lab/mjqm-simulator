//
// Created by mccio on 24/01/25.
//

#ifndef DETERMINISTIC_H
#define DETERMINISTIC_H

#include "../sampler.h"

#include <memory>

class deterministic : public sampler
{
public:
    explicit deterministic(const double mean) : mean(mean) {}

private:
    double mean;
    double variance = 0;

public:
    double sample() override { return mean; }

    static std::unique_ptr<sampler> with_mean(double mean) { return std::make_unique<deterministic>(mean); }

    explicit operator std::string() const override { return "deterministic (mean=" + std::to_string(mean) + " => variance=0)"; }
};

#endif // DETERMINISTIC_H
