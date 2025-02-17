//
// Created by Marco Ciotola on 23/01/25.
//

#include <iostream>
#include <mjqm-simulator/experiment_stats.h>
#include <vector>

std::ostream& operator<<(std::ostream& os, const Stat<Confidence_inter>& m) {
    os << m.value;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Stat<bool>& m) {
    os << m.value << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExperimentStats& m) {
    for (auto& cs : m.class_stats) {
        os << cs;
    }
    os << m.wasted;
    os << m.utilisation;
    os << m.occupancy_tot;
    os << m.wait_tot;
    os << m.wait_var_tot;
    os << m.resp_tot;
    os << m.resp_var_tot;
    os << m.window_size;
    os << m.violations;
    os << m.timings_tot;
    os << m.big_seq_avg_len;
    os << m.small_seq_avg_len;
    os << m.big_seq_avg_dur;
    os << m.small_seq_avg_dur;
    os << m.big_seq_amount;
    os << m.small_seq_amount;
    os << m.phase_two_dur;
    os << m.phase_three_dur;
    return os;
}

void ExperimentStats::add_headers(std::vector<std::string>& headers, std::vector<unsigned int>&) const {
    add_headers(headers);
}

void ExperimentStats::add_headers(std::vector<std::string>& headers) const {
    for (auto& cs : class_stats) {
        cs.add_headers(headers);
    }
    wasted.add_headers(headers);
    utilisation.add_headers(headers);
    occupancy_tot.add_headers(headers);
    wait_tot.add_headers(headers);
    wait_var_tot.add_headers(headers);
    resp_tot.add_headers(headers);
    resp_var_tot.add_headers(headers);
    window_size.add_headers(headers);
    violations.add_headers(headers);
    timings_tot.add_headers(headers);
    big_seq_avg_len.add_headers(headers);
    small_seq_avg_len.add_headers(headers);
    big_seq_avg_dur.add_headers(headers);
    small_seq_avg_dur.add_headers(headers);
    big_seq_amount.add_headers(headers);
    small_seq_amount.add_headers(headers);
    phase_two_dur.add_headers(headers);
    phase_three_dur.add_headers(headers);
}
