//
// Created by mccio on 23/01/25.
//

#include "confidence_intervals.h"

#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <cmath>
#include <vector>

Confidence_inter compute_interval_student(const std::vector<double>& rep, const double confidence) {
    double mean = 0.0;
    double stdv = 0.0;

    for (int i = 0; i < rep.size(); i++)
        mean += rep[i];
    mean /= static_cast<double>(rep.size());

    for (int i = 0; i < rep.size(); i++)
        stdv += std::pow(rep[i] - mean, 2);

    stdv = std::sqrt(stdv);
    if (rep.size() > 1)
        stdv /= static_cast<double>(rep.size()) - 1;
    else
        stdv = 0.0;

    const boost::math::students_t dist(static_cast<double>(rep.size()) - 1);
    const double t = boost::math::quantile(boost::math::complement(dist, confidence / 2.0));
    const double delta = t * stdv / std::sqrt(static_cast<double>(rep.size()));

    return Confidence_inter{mean - delta, mean + delta, mean};
}

Confidence_inter compute_interval_class_student(const std::vector<std::vector<double>>& rep, const int cl,
                                                const double confidence) {

    double mean = 0.0;
    double stdv = 0.0;

    for (int i = 0; i < rep.size(); i++)
        mean += rep[i][cl];
    mean /= static_cast<double>(rep.size());

    for (int i = 0; i < rep.size(); i++)
        stdv += std::pow(rep[i][cl] - mean, 2);

    stdv = std::sqrt(stdv);
    if (rep.size() > 1)
        stdv /= static_cast<double>(rep.size()) - 1;
    else
        stdv = 0.0;

    const boost::math::students_t dist(static_cast<double>(rep.size()) - 1);
    const double t = boost::math::quantile(boost::math::complement(dist, confidence / 2.0));
    const double delta = t * stdv / std::sqrt(static_cast<double>(rep.size()));

    return Confidence_inter{mean - delta, mean + delta, mean};
}

Confidence_inter compute_interval_class_chi(const std::vector<std::vector<double>>& rep, const int cl,
                                            const double confidence) {

    double mean = 0.0;
    double stdv = 0.0;

    for (int i = 0; i < rep.size(); i++)
        mean += rep[i][cl];
    mean /= static_cast<double>(rep.size());

    for (int i = 0; i < rep.size(); i++)
        stdv += std::pow(rep[i][cl] - mean, 2);

    stdv = std::sqrt(stdv);
    if (rep.size() > 1)
        stdv /= static_cast<double>(rep.size()) - 1;
    else
        stdv = 0.0;

    const boost::math::chi_squared dist(static_cast<double>(rep.size()) - 1);
    const double t_right = boost::math::quantile(boost::math::complement(dist, confidence / 2.0));
    const double t_left = boost::math::quantile(boost::math::complement(dist, (1.0 - confidence) / 2.0));
    const double delta_right = std::sqrt(stdv * (static_cast<double>(rep.size()) - 1) / t_right);
    const double delta_left = std::sqrt(stdv * (static_cast<double>(rep.size()) - 1) / t_left);

    return Confidence_inter{mean - delta_left, mean + delta_right, mean};
}
