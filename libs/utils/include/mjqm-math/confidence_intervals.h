//
// Created by Marco Ciotola on 23/01/25.
//

#ifndef MJQM_MATH_CONFIDENCE_INTERVALS_H
#define MJQM_MATH_CONFIDENCE_INTERVALS_H

#include <iostream>
#include <vector>

struct Confidence_inter {
    double min;
    double max;
    double mean;

    friend std::ostream& operator<<(std::ostream& os, Confidence_inter const& m) {
        return os << m.mean << ";" << "[" << m.min << ", " << m.max << "]";
    }

    friend Confidence_inter operator+(Confidence_inter const& m, Confidence_inter const& o) {
        return Confidence_inter{m.min + o.min, m.max + o.max, m.mean + o.mean};
    }
};

Confidence_inter compute_interval_student(const std::vector<double>& rep, double confidence);

Confidence_inter compute_interval_chi(const std::vector<double>& rep, double confidence);

#endif // MJQM_MATH_CONFIDENCE_INTERVALS_H
