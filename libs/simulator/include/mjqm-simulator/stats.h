#ifndef STATS_H
#define STATS_H

#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <mjqm-math/confidence_intervals.h>

class Simulator;

class Stat {
    const std::string name;
    const double confidence;
    Confidence_inter interval{};
    const std::vector<double>& perRunValues;

public:
    Stat(std::string name, const std::vector<double>& perRunValues, double confidence = 0.95) :
        name(std::move(name)), perRunValues(perRunValues), confidence(confidence) {}
    virtual void compute_at_end() { interval = compute_interval_student(perRunValues, confidence); }
    virtual void add_headers(std::vector<std::string>& headers) const {
        headers.push_back(name);
        headers.push_back(name + " ConfInt");
    }
    std::ostream& operator<<(std::ostream& os) const { return os << interval; }
};

class StatFactory {
    const std::string name;
    const std::string description;
    Stat (*factory)(const std::string&, Simulator&);

public:
    StatFactory(std::string name, std::string description, Stat (*factory)(const std::string&, Simulator&)) :
        name(std::move(name)), description(std::move(description)), factory(factory) {}
    Stat operator()(Simulator& sim) const { return factory(name, sim); }
};

class ExperimentStatsNew {
private:
    inline static std::map<std::string, StatFactory> known_stats = {};

public:
    static bool register_stat(const std::string& name, const std::string& description,
                              Stat (*factory)(const std::string&, Simulator&)) {
        std::string name_lower(name);
        for (char& c : name_lower) {
            if (c == ' ')
                c = '_';
            else
                c = std::tolower(c);
        }
        return known_stats
                   .emplace(std::piecewise_construct, std::forward_as_tuple(name),
                            std::forward_as_tuple(name, description, factory))
                   .second ||
            known_stats
                .emplace(std::piecewise_construct, std::forward_as_tuple(name_lower),
                         std::forward_as_tuple(name, description, factory))
                .second;
    }
    static bool register_stat(const std::string& name, Stat (*factory)(const std::string&, Simulator&)) {
        return register_stat(name, name, factory);
    }
    static std::vector<std::string> knownStats() {
        std::vector<std::string> names;
        names.reserve(known_stats.size());
        for (const auto& [name, _] : known_stats) {
            names.push_back(name);
        }
        return names;
    }

private:
    std::vector<Stat> stats;

public:
    ExperimentStatsNew(const std::vector<std::string>& names, Simulator& sim) {
        for (auto& name : names) {
            stats.emplace_back(known_stats.at(name)(sim));
        }
    }
    void compute_at_end() {
        for (auto& stat : stats) {
            stat.compute_at_end();
        }
    }
    void add_headers(std::vector<std::string>& headers) const {
        for (const auto& stat : stats) {
            stat.add_headers(headers);
        }
    }
};

#endif // STATS_H
