//
// Created by Marco Ciotola on 23/01/25.
//

#include <mjqm-simulator/experiment_stats.h>

std::ostream& operator<<(std::ostream& os, ExperimentStats const& m) {
    for (int c = 0; c < m.occupancy_buf.size(); c++) {

        os << m.occupancy_buf.at(c);
        os << m.occupancy_ser.at(c);
        os << m.occupancy_buf.at(c) + m.occupancy_ser.at(c);
        os << m.wait_time.at(c);
        os << m.wait_time_var.at(c);
        os << m.throughput.at(c);
        os << m.resp_time.at(c);
        os << m.resp_time_var.at(c);
        os << m.preemption_avg.at(c);
        os << m.warnings.at(c) << ";";
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

void ExperimentStats::add_headers(std::vector<std::string>& headers, std::vector<unsigned int>& sizes) const {
    for (unsigned int ts : sizes) {
        headers.insert(headers.end(),
                       {
                           "T" + std::to_string(ts) + " Queue",
                           "T" + std::to_string(ts) + " Queue ConfInt",
                           "T" + std::to_string(ts) + " Service",
                           "T" + std::to_string(ts) + " Service ConfInt",
                           "T" + std::to_string(ts) + " System",
                           "T" + std::to_string(ts) + " System ConfInt",
                           "T" + std::to_string(ts) + " Waiting",
                           "T" + std::to_string(ts) + " Waiting ConfInt",
                           "T" + std::to_string(ts) + " Waiting Variance",
                           "T" + std::to_string(ts) + " Waiting Variance ConfInt",
                           "T" + std::to_string(ts) + " Throughput",
                           "T" + std::to_string(ts) + " Throughput ConfInt",
                           "T" + std::to_string(ts) + " RespTime",
                           "T" + std::to_string(ts) + " RespTime ConfInt",
                           "T" + std::to_string(ts) + " RespTime Variance",
                           "T" + std::to_string(ts) + " RespTime Variance ConfInt",
                           "T" + std::to_string(ts) + " Preemption",
                           "T" + std::to_string(ts) + " Preemption ConfInt",
                           "T" + std::to_string(ts) + " Stability Check",
                       });
    }

    headers.insert(headers.end(),
                   {
                       "Wasted Servers",
                       "Wasted Servers ConfInt",
                       "Utilisation",
                       "Utilisation ConfInt",
                       "Queue Total",
                       "Queue Total ConfInt",
                       "WaitTime Total",
                       "WaitTime Total ConfInt",
                       "WaitTime Variance",
                       "WaitTime Variance ConfInt",
                       "RespTime Total",
                       "RespTime Total ConfInt",
                       "RespTime Variance",
                       "RespTime Variance ConfInt",
                       "Window Size",
                       "Window Size ConfInt",
                       "FIFO Violations",
                       "FIFO Violations ConfInt",
                       "Run Duration",
                       "Run Duration ConfInt",
                       "Big Sequence Length",
                       "Big Sequence Length ConfInt",
                       "Small Sequence Length",
                       "Small Sequence Length ConfInt",
                       "Big Sequence Duration",
                       "Big Sequence Duration ConfInt",
                       "Small Sequence Duration",
                       "Small Sequence Duration ConfInt",
                       "Big Sequence Amount",
                       "Big Sequence Amount ConfInt",
                       "Small Sequence Amount",
                       "Small Sequence Amount ConfInt",
                       "Phase Two Duration",
                       "Phase Two Duration ConfInt",
                       "Phase Three Duration",
                       "Phase Three Duration ConfInt",
                   });
}
