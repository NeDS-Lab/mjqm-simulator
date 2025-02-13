//
// Created by Marco Ciotola on 23/01/25.
//

#ifndef EXPERIMENT_STATS_H
#define EXPERIMENT_STATS_H

// ReSharper disable once CppUnusedIncludeDirective
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <mjqm-math/confidence_intervals.h>
#include <vector>

struct ExperimentStats {
    std::vector<Confidence_inter> occupancy_buf; // out: occupancy buffer per class
    std::vector<Confidence_inter> occupancy_ser; // out: occupancy servers per class
    std::vector<Confidence_inter> throughput; // out: throughput per class
    std::vector<Confidence_inter> wait_time; // output: waiting time per class
    std::vector<Confidence_inter> wait_time_var; // output: waiting time variance per class
    std::vector<Confidence_inter> resp_time; // output: response time per class
    std::vector<Confidence_inter> resp_time_var; // output: waiting time variance per class
    std::vector<bool> warnings; // out: warning for class stability
    std::vector<Confidence_inter> preemption_avg;
    Confidence_inter wasted{}; // out: wasted servers
    Confidence_inter violations{}; // out: fifo violations counter
    Confidence_inter utilisation{}; // out: wasted servers
    Confidence_inter occupancy_tot{}; // out: total queue length
    Confidence_inter wait_tot{}; // out: total waiting time
    Confidence_inter wait_var_tot{}; // out: total waiting time variance
    Confidence_inter resp_tot{}; // out: total response time
    Confidence_inter resp_var_tot{}; // out: total response time
    Confidence_inter timings_tot{}; // out: average run duration
    Confidence_inter big_seq_avg_len{}; // out: average length of big service sequence
    Confidence_inter small_seq_avg_len{}; // out: average length of big service sequence
    Confidence_inter big_seq_avg_dur{}; // out: average length of big service sequence
    Confidence_inter small_seq_avg_dur{}; // out: average length of big service sequence
    Confidence_inter big_seq_amount{}; // out: amount of big service sequence
    Confidence_inter small_seq_amount{}; // out: amount of small service sequence
    Confidence_inter phase_two_dur{}; // out: duration of phase two
    Confidence_inter phase_three_dur{}; // out: duration of phase three
    Confidence_inter window_size{};

    friend std::ostream& operator<<(std::ostream& os, ExperimentStats const& m);
    void add_headers(std::vector<std::string>& headers, std::vector<unsigned int>& sizes) const;

    template <class Archive>
    void serialize(Archive& ar, unsigned int) {
        ar & occupancy_buf;
        ar & occupancy_ser;
        ar & throughput;
        ar & wait_time;
        ar & wait_time_var;
        ar & resp_time;
        ar & resp_time_var;
        ar & warnings;
        ar & preemption_avg;
        ar & wasted;
        ar & violations;
        ar & utilisation;
        ar & occupancy_tot;
        ar & wait_tot;
        ar & wait_var_tot;
        ar & resp_tot;
        ar & resp_var_tot;
        ar & timings_tot;
        ar & big_seq_avg_len;
        ar & small_seq_avg_len;
        ar & big_seq_avg_dur;
        ar & small_seq_avg_dur;
        ar & big_seq_amount;
        ar & small_seq_amount;
        ar & phase_two_dur;
        ar & phase_three_dur;
        ar & window_size;
    }
};

#endif // EXPERIMENT_STATS_H
