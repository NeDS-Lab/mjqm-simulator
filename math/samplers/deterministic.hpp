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
};

#endif // DETERMINISTIC_H
