//
// Created by Marco Ciotola on 30/03/25.
//

#ifndef MEAN_VAR_H
#define MEAN_VAR_H

#include <cmath>
#include <list>
#include <vector>

std::pair<double, double> mean_var(const std::vector<double>& values);
std::pair<double, double> mean_var(const std::list<double>& values);

#endif // MEAN_VAR_H
