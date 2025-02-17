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
#include <vector>

#include <mjqm-math/confidence_intervals.h>

template <typename CONTENT>
class Stat {
public:
    const std::string name;
    const bool has_confidence_interval;
    CONTENT value;

    Stat(std::string name, bool has_confidence_interval) :
        name{std::move(name)}, has_confidence_interval{has_confidence_interval} {}

    void add_headers(std::vector<std::string>& headers) const {
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

class ClassStats {
public:
    const std::string name;
    Stat<Confidence_inter> occupancy_buf; // out: occupancy buffer per class
    Stat<Confidence_inter> occupancy_ser; // out: occupancy servers per class
    Stat<Confidence_inter> occupancy_system; // out: occupancy in the system per class
    Stat<Confidence_inter> wait_time; // output: waiting time per class
    Stat<Confidence_inter> wait_time_var; // output: waiting time variance per class
    Stat<Confidence_inter> throughput; // out: throughput per class
    Stat<Confidence_inter> resp_time; // output: response time per class
    Stat<Confidence_inter> resp_time_var; // output: waiting time variance per class
    Stat<Confidence_inter> preemption_avg;
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
public:
    std::vector<ClassStats> class_stats;
    Stat<Confidence_inter> wasted{"Wasted Servers", true}; // out: wasted servers
    Stat<Confidence_inter> utilisation{"Utilisation", true}; // out: wasted servers
    Stat<Confidence_inter> occupancy_tot{"Queue Total", true}; // out: total queue length
    Stat<Confidence_inter> wait_tot{"WaitTime Total", true}; // out: total waiting time
    Stat<Confidence_inter> wait_var_tot{"WaitTime Variance", true}; // out: total waiting time variance
    Stat<Confidence_inter> resp_tot{"RespTime Total", true}; // out: total response time
    Stat<Confidence_inter> resp_var_tot{"RespTime Variance", true}; // out: total response time
    Stat<Confidence_inter> window_size{"Window Size", true};
    Stat<Confidence_inter> violations{"FIFO Violations", true}; // out: fifo violations counter
    Stat<Confidence_inter> timings_tot{"Run Duration", true}; // out: average run duration
    Stat<Confidence_inter> big_seq_avg_len{"Big Sequence Length", true}; // out: average length of big service sequence
    Stat<Confidence_inter> small_seq_avg_len{"Small Sequence Length",
                                             true}; // out: average length of big service sequence
    Stat<Confidence_inter> big_seq_avg_dur{"Big Sequence Duration",
                                           true}; // out: average length of big service sequence
    Stat<Confidence_inter> small_seq_avg_dur{"Small Sequence Duration",
                                             true}; // out: average length of big service sequence
    Stat<Confidence_inter> big_seq_amount{"Big Sequence Amount", true}; // out: amount of big service sequence
    Stat<Confidence_inter> small_seq_amount{"Small Sequence Amount", true}; // out: amount of small service sequence
    Stat<Confidence_inter> phase_two_dur{"Phase Two Duration", true}; // out: duration of phase two
    Stat<Confidence_inter> phase_three_dur{"Phase Three Duration", true}; // out: duration of phase three

    friend std::ostream& operator<<(std::ostream& os, ExperimentStats const& m);
    void add_headers(std::vector<std::string>& headers, std::vector<unsigned int>& sizes) const;
    void add_headers(std::vector<std::string>& headers) const;
};

#endif // EXPERIMENT_STATS_H
