//
// Created by Marco Ciotola on 23/01/25.
//

#include <cmath>
#include <vector>

#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/distributions/students_t.hpp>

#include <mjqm-math/confidence_intervals.h>
#include <mjqm-math/mean_var.h>

Confidence_inter compute_interval_student(const std::vector<double>& rep, const double confidence) {
    const auto size = static_cast<double>(rep.size());
    double mean = 0.0;
    double stdv = 0.0;

    auto mv = mean_var(rep);
    mean = mv.first;
    stdv = sqrt(mv.second);

    if (rep.size() > 1)
        stdv /= size - 1;
    else
        stdv = 0.0;

    const boost::math::students_t dist(size - 1);
    const double t = boost::math::quantile(boost::math::complement(dist, confidence / 2.0));
    const double delta = t * stdv / sqrt(size);

    return Confidence_inter{mean - delta, mean + delta, mean};
}

Confidence_inter compute_interval_chi(const std::vector<double>& rep, const double confidence) {
    const auto size = static_cast<double>(rep.size());
    double mean = 0.0;
    double stdv = 0.0;

    auto mv = mean_var(rep);
    mean = mv.first;
    stdv = sqrt(mv.second);

    if (rep.size() > 1)
        stdv /= size - 1;
    else
        stdv = 0.0;

    const boost::math::chi_squared dist(size - 1);
    const double t_right = boost::math::quantile(boost::math::complement(dist, confidence / 2.0));
    const double t_left = boost::math::quantile(boost::math::complement(dist, 1.0 - confidence / 2.0));
    const double delta_right = sqrt(stdv * (size - 1) / t_right);
    const double delta_left = sqrt(stdv * (size - 1) / t_left);

    return Confidence_inter{mean - delta_left, mean + delta_right, mean};
}
