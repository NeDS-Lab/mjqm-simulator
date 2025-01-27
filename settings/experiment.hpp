//
// Created by mccio on 24/01/25.
//

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <vector>
#include <string>


struct Experiment
{
    std::vector<double> l;
    std::vector<double> u;
    std::vector<int> s;
    int w;
    int n;
    int sm;
    std::string logf;
};


#endif // EXPERIMENT_H