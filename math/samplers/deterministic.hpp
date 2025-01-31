//
// Created by mccio on 24/01/25.
//

#ifndef DETERMINISTIC_H
#define DETERMINISTIC_H

#include "../sampler.h"

#include <memory>

class deterministic : public sampler {
public:
    explicit deterministic(const double value) : value(value) {}

private:
    const double value;
    const double variance = 0;

public:
    double sample() override { return value; }

    static std::unique_ptr<sampler> with_value(double value) { return std::make_unique<deterministic>(value); }

    std::unique_ptr<sampler> clone(std::shared_ptr<std::mt19937_64>) const override {
        return std::make_unique<deterministic>(value);
    }

    explicit operator std::string() const override {
        return "deterministic (mean=" + std::to_string(value) + " => variance=0)";
    }
};

#endif // DETERMINISTIC_H
