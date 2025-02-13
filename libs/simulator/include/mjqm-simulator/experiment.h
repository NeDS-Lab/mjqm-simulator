//
// Created by Marco Ciotola on 24/01/25.
//

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <string>
#include <vector>

struct Experiment {
    std::vector<double> l;
    std::vector<double> u;
    std::vector<unsigned int> s;
    int w;
    int n;
    int sm;
    std::string logf;
    template <class Archive>
    void serialize(Archive& ar, unsigned int);
};
template <class Archive>
void Experiment::serialize(Archive& ar, const unsigned int) {
    ar & l;
    ar & u;
    ar & s;
    ar & w;
    ar & n;
    ar & sm;
    ar & logf;
}

#endif // EXPERIMENT_H
