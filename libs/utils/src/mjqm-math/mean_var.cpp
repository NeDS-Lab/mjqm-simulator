//
// Created by Marco Ciotola on 30/03/25.
//

#include <cmath>
#include <list>
#include <numeric>
#include <vector>

#include <mjqm-math/mean_var.h>

template <typename InputIterator>
std::pair<double, double> mean_var(InputIterator _first, InputIterator _last, double _init, double size) {
    double mean = 0;
    double var = 0;

    mean = std::accumulate(_first, _last, 0.0);
    mean /= size;
    var = std::accumulate(_first, _last, 0.0,
                          [&mean](double acc, const double& value) { return acc + std::pow(value - mean, 2); });
    var /= size;

    return {mean, var};
}

std::pair<double, double> mean_var(const std::vector<double>& values) {
    return mean_var(values.begin(), values.end(), 0.0, static_cast<double>(values.size()));
}

std::pair<double, double> mean_var(const std::list<double>& values) {
    return mean_var(values.begin(), values.end(), 0.0, static_cast<double>(values.size()));
}
