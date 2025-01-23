//
// Created by mccio on 23/01/25.
//

#ifndef CONFIDENCE_INTERVALS_H
#define CONFIDENCE_INTERVALS_H

#include <iostream>
#include <vector>

struct Confidence_inter
{
    double min;
    double max;
    double mean;

    friend std::ostream& operator<<(std::ostream& os, Confidence_inter const& m)
    {
        return os << m.mean << ";" << "[" << m.min << ", " << m.max << "]" << ";";
    }

    friend Confidence_inter operator+(Confidence_inter const& m, Confidence_inter const& o)
    {
        return Confidence_inter{m.min + o.min, m.max + o.max, m.mean + o.mean};
    }

    Confidence_inter& operator=(Confidence_inter const& o) = default;
};


Confidence_inter compute_interval_student(const std::vector<double>& rep, double confidence);

Confidence_inter compute_interval_class_student(const std::vector<std::vector<double>>& rep, int cl, double confidence);

Confidence_inter compute_interval_class_chi(const std::vector<std::vector<double>>& rep, int cl, double confidence);


#endif // CONFIDENCE_INTERVALS_H
