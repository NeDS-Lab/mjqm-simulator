#include <mjqm-simulator/simulator.h>
#include <mjqm-simulator/stats.h>

Stat wasted_servers(const std::string& name, Simulator& sim) { return {name, sim.rep_waste}; }
Stat utilisation(const std::string& name, Simulator& sim) { return {name, sim.rep_util}; }
Stat occupancy_total(const std::string& name, Simulator& sim) { return {name, sim.rep_tot_buf}; }
Stat wait_time_total(const std::string& name, Simulator& sim) { return {name, sim.rep_tot_wait}; }
Stat wait_time_variance_total(const std::string& name, Simulator& sim) { return {name, sim.rep_tot_wait_var}; }
Stat response_time_total(const std::string& name, Simulator& sim) { return {name, sim.rep_tot_resp}; }
Stat response_time_variance_total(const std::string& name, Simulator& sim) { return {name, sim.rep_tot_resp_var}; }
Stat timings_total(const std::string& name, Simulator& sim) { return {name, sim.rep_timings}; }
Stat big_sequence_average_length(const std::string& name, Simulator& sim) { return {name, sim.rep_big_seq_avg_len}; }
Stat small_sequence_average_length(const std::string& name, Simulator& sim) { return {name, sim.rep_small_seq_avg_len}; }
Stat big_sequence_average_duration(const std::string& name, Simulator& sim) { return {name, sim.rep_big_seq_avg_dur}; }
Stat small_sequence_average_duration(const std::string& name, Simulator& sim) { return {name, sim.rep_small_seq_avg_dur}; }
Stat big_sequence_amount(const std::string& name, Simulator& sim) { return {name, sim.rep_big_seq_amount}; }
Stat small_sequence_amount(const std::string& name, Simulator& sim) { return {name, sim.rep_small_seq_amount}; }
Stat phase_two_duration(const std::string& name, Simulator& sim) { return {name, sim.rep_phase_two_duration}; }
Stat phase_three_duration(const std::string& name, Simulator& sim) { return {name, sim.rep_phase_three_duration}; }

const bool REGISTERED_STATS = ExperimentStatsNew::register_stat("Wasted Servers", wasted_servers) &&
    ExperimentStatsNew::register_stat("Utilisation", utilisation) &&
    ExperimentStatsNew::register_stat("Total Occupancy", occupancy_total) &&
    ExperimentStatsNew::register_stat("Total Waiting Time", wait_time_total) &&
    ExperimentStatsNew::register_stat("Total Waiting Time Variance", wait_time_variance_total) &&
    ExperimentStatsNew::register_stat("Total Response Time", response_time_total) &&
    ExperimentStatsNew::register_stat("Total Response Time Variance", response_time_variance_total) &&
    ExperimentStatsNew::register_stat("Total Timings", timings_total) &&
    ExperimentStatsNew::register_stat("Big Sequence Average Length", big_sequence_average_length) &&
    ExperimentStatsNew::register_stat("Small Sequence Average Length", small_sequence_average_length) &&
    ExperimentStatsNew::register_stat("Big Sequence Average Duration", big_sequence_average_duration) &&
    ExperimentStatsNew::register_stat("Small Sequence Average Duration", small_sequence_average_duration) &&
    ExperimentStatsNew::register_stat("Big Sequence Amount", big_sequence_amount) &&
    ExperimentStatsNew::register_stat("Small Sequence Amount", small_sequence_amount) &&
    ExperimentStatsNew::register_stat("Phase Two Duration", phase_two_duration) &&
    ExperimentStatsNew::register_stat("Phase Three Duration", phase_three_duration);
