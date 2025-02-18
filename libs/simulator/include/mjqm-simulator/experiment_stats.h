//
// Created by Marco Ciotola on 23/01/25.
//

#ifndef EXPERIMENT_STATS_H
#define EXPERIMENT_STATS_H

#include <cwchar>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <mjqm-math/confidence_intervals.h>

template <typename CONTENT = Confidence_inter>
class Stat {
public:
    const std::string name;
    const bool has_confidence_interval;
    CONTENT value;
    bool visible = true;

    Stat(std::string name, bool has_confidence_interval) :
        name{std::move(name)}, has_confidence_interval{has_confidence_interval} {}

    void add_headers(std::vector<std::string>& headers) const {
        if (!visible) {
            return;
        }
        headers.insert(headers.end(), name);
        if (has_confidence_interval) {
            headers.insert(headers.end(), name + " ConfInt");
        }
    }

    Stat& operator=(CONTENT const& value) {
        this->value = value;
        return *this;
    }
    CONTENT operator+(Stat<CONTENT> const& that) const { return this->value + that.value; }
};
std::ostream& operator<<(std::ostream& os, const Stat<Confidence_inter>& m);
std::ostream& operator<<(std::ostream& os, const Stat<bool>& m);
std::ostream& operator<<(std::ostream& os, const Stat<std::variant<double, long, std::string>>& m);

class ClassStats {
public:
    const std::string name;
    Stat<> occupancy_buf; // out: occupancy buffer per class
    Stat<> occupancy_ser; // out: occupancy servers per class
    Stat<> occupancy_system; // out: occupancy in the system per class
    Stat<> wait_time; // output: waiting time per class
    Stat<> wait_time_var; // output: waiting time variance per class
    Stat<> throughput; // out: throughput per class
    Stat<> resp_time; // output: response time per class
    Stat<> resp_time_var; // output: waiting time variance per class
    Stat<> preemption_avg;
    Stat<bool> warnings; // out: warning for class stability

    explicit ClassStats(const std::string& name) :
        name{name}, occupancy_buf{"T" + name + " Queue", true}, occupancy_ser{"T" + name + " Service", true},
        occupancy_system{"T" + name + " System", true}, wait_time{"T" + name + " Waiting", true},
        wait_time_var{"T" + name + " Waiting Variance", true}, throughput{"T" + name + " Throughput", true},
        resp_time{"T" + name + " RespTime", true}, resp_time_var{"T" + name + " RespTime Variance", true},
        preemption_avg{"T" + name + " Preemption", true}, warnings{"T" + name + " Stability Check", false} {}

    void add_headers(std::vector<std::string>& headers) const {
        occupancy_buf.add_headers(headers);
        occupancy_ser.add_headers(headers);
        occupancy_system.add_headers(headers);
        wait_time.add_headers(headers);
        wait_time_var.add_headers(headers);
        throughput.add_headers(headers);
        resp_time.add_headers(headers);
        resp_time_var.add_headers(headers);
        preemption_avg.add_headers(headers);
        warnings.add_headers(headers);
    }
    friend std::ostream& operator<<(std::ostream& os, ClassStats const& m) {
        os << m.occupancy_buf;
        os << m.occupancy_ser;
        os << m.occupancy_buf + m.occupancy_ser; // occupancy system
        os << m.wait_time;
        os << m.wait_time_var;
        os << m.throughput;
        os << m.resp_time;
        os << m.resp_time_var;
        os << m.preemption_avg;
        os << m.warnings;
        return os;
    }
};

class ExperimentStats {
    std::vector<Stat<std::variant<double, long, std::string>>> pivot_values{};

public:
    std::vector<ClassStats> class_stats{};
    Stat<> wasted{"Wasted Servers", true}; // out: wasted servers
    Stat<> utilisation{"Utilisation", true}; // out: wasted servers
    Stat<> occupancy_tot{"Queue Total", true}; // out: total queue length
    Stat<> wait_tot{"WaitTime Total", true}; // out: total waiting time
    Stat<> wait_var_tot{"WaitTime Variance", true}; // out: total waiting time variance
    Stat<> resp_tot{"RespTime Total", true}; // out: total response time
    Stat<> resp_var_tot{"RespTime Variance", true}; // out: total response time
    Stat<> window_size{"Window Size", true};
    Stat<> violations{"FIFO Violations", true}; // out: fifo violations counter
    Stat<> timings_tot{"Run Duration", true}; // out: average run duration
    Stat<> big_seq_avg_len{"Big Sequence Length", true}; // out: average length of big service sequence
    Stat<> small_seq_avg_len{"Small Sequence Length", true}; // out: average length of big service sequence
    Stat<> big_seq_avg_dur{"Big Sequence Duration", true}; // out: average length of big service sequence
    Stat<> small_seq_avg_dur{"Small Sequence Duration", true}; // out: average length of big service sequence
    Stat<> big_seq_amount{"Big Sequence Amount", true}; // out: amount of big service sequence
    Stat<> small_seq_amount{"Small Sequence Amount", true}; // out: amount of small service sequence
    Stat<> phase_two_dur{"Phase Two Duration", true}; // out: duration of phase two
    Stat<> phase_three_dur{"Phase Three Duration", true}; // out: duration of phase three

    friend std::ostream& operator<<(std::ostream& os, ExperimentStats const& m);
    void add_headers(std::vector<std::string>& headers, std::vector<unsigned int>& sizes) const;
    void add_headers(std::vector<std::string>& headers) const;

    template <typename VAL_TYPE>
    bool add_pivot_value(const std::string& name, const VAL_TYPE& value) {
        pivot_values.emplace_back(name, false);
        pivot_values.back() = std::variant<double, long, std::string>(value);
        return true;
    }
};

#endif // EXPERIMENT_STATS_H
