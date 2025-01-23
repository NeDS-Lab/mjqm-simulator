//
// Created by mccio on 23/01/25.
//

#include "experiment_stats.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, ExperimentStats const& m)
{

    for (int c = 0; c < m.occupancy_buf.size(); c++)
    {

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
