//
// Created by Marco Ciotola on 23/01/25.
//

#include <iostream>
#include <variant>
#include <vector>

#include <mjqm-simulator/experiment_stats.h>

std::ostream& operator<<(std::ostream& os, const Stat<Confidence_inter>& m) {
    if (!m.visible) {
        return os;
    }
    if (m.has_confidence_interval)
        os << m.value;
    else
        os << m.value.mean << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Stat<bool>& m) {
    if (!m.visible) {
        return os;
    }
    os << m.value << ";";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Stat<VariantStat>& m) {
    if (!m.visible) {
        return os;
    }
    m.value.index() == 0       ? os << std::get<double>(m.value) << ";"
        : m.value.index() == 1 ? os << std::get<long>(m.value) << ";"
        : m.value.index() == 2 ? os << std::get<std::string>(m.value) << ";"
                               : os << "N/A;";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExperimentStats& m) {
    for (auto& pv : m.pivot_values) {
        os << pv;
    }
    for (auto& cv : m.custom_values) {
        os << cv;
    }
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
    for (auto& pv : pivot_values) {
        pv.add_headers(headers);
    }
    for (auto& cv : custom_values) {
        cv.add_headers(headers);
    }
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

bool ExperimentStats::set_computed_columns_visibility(bool visible) {
    for (auto& cls : class_stats) {
        cls.set_computed_columns_visibility(visible);
    }
    wasted.visible = visible;
    utilisation.visible = visible;
    occupancy_tot.visible = visible;
    wait_tot.visible = visible;
    wait_var_tot.visible = visible;
    resp_tot.visible = visible;
    resp_var_tot.visible = visible;
    window_size.visible = visible;
    violations.visible = visible;
    timings_tot.visible = visible;
    big_seq_avg_len.visible = visible;
    small_seq_avg_len.visible = visible;
    big_seq_avg_dur.visible = visible;
    small_seq_avg_dur.visible = visible;
    big_seq_amount.visible = visible;
    small_seq_amount.visible = visible;
    phase_two_dur.visible = visible;
    phase_three_dur.visible = visible;
    return true;
}
bool ExperimentStats::set_column_visibility(const std::string& column, bool visible) {
    for (auto& pv : pivot_values) {
        if (column == "pivots") {
            pv.visible = visible;
        } else if (pv.name == column) {
            pv.visible = visible;
            return true;
        }
    }
    if (column == "pivots") {
        return true;
    }
    for (auto& cv : custom_values) {
        if (cv.name == column) {
            cv.visible = visible;
            return true;
        }
    }
    if (wasted.name == column) {
        wasted.visible = visible;
        return true;
    }
    if (utilisation.name == column) {
        utilisation.visible = visible;
        return true;
    }
    if (occupancy_tot.name == column) {
        occupancy_tot.visible = visible;
        return true;
    }
    if (wait_tot.name == column) {
        wait_tot.visible = visible;
        return true;
    }
    if (wait_var_tot.name == column) {
        wait_var_tot.visible = visible;
        return true;
    }
    if (resp_tot.name == column) {
        resp_tot.visible = visible;
        return true;
    }
    if (resp_var_tot.name == column) {
        resp_var_tot.visible = visible;
        return true;
    }
    if (window_size.name == column) {
        window_size.visible = visible;
        return true;
    }
    if (violations.name == column) {
        violations.visible = visible;
        return true;
    }
    if (timings_tot.name == column) {
        timings_tot.visible = visible;
        return true;
    }
    if (big_seq_avg_len.name == column) {
        big_seq_avg_len.visible = visible;
        return true;
    }
    if (small_seq_avg_len.name == column) {
        small_seq_avg_len.visible = visible;
        return true;
    }
    if (big_seq_avg_dur.name == column) {
        big_seq_avg_dur.visible = visible;
        return true;
    }
    if (small_seq_avg_dur.name == column) {
        small_seq_avg_dur.visible = visible;
        return true;
    }
    if (big_seq_amount.name == column) {
        big_seq_amount.visible = visible;
        return true;
    }
    if (small_seq_amount.name == column) {
        small_seq_amount.visible = visible;
        return true;
    }
    if (phase_two_dur.name == column) {
        phase_two_dur.visible = visible;
        return true;
    }
    if (phase_three_dur.name == column) {
        phase_three_dur.visible = visible;
        return true;
    }
    return false;
}
bool ExperimentStats::set_class_column_visibility(const std::string& column, bool visible, const std::string& cls) {
    bool found = false;
    bool res = true;
    for (auto& class_stat : class_stats) {
        if (class_stat.name == cls || cls == "*") {
            found = true;
            res = class_stat.set_column_visibility(column, visible) && res;
        }
    }
    return found ? res : false;
}

bool ClassStats::set_computed_columns_visibility(bool visible) {
    occupancy_buf.visible = visible;
    occupancy_ser.visible = visible;
    occupancy_system.visible = visible;
    wait_time.visible = visible;
    wait_time_var.visible = visible;
    throughput.visible = visible;
    resp_time.visible = visible;
    resp_time_var.visible = visible;
    preemption_avg.visible = visible;
    warnings.visible = visible;
    return true;
}
bool ClassStats::set_column_visibility(const std::string& column, bool visible) {
    if (occupancy_buf.name == column) {
        occupancy_buf.visible = visible;
        return true;
    }
    if (occupancy_ser.name == column) {
        occupancy_ser.visible = visible;
        return true;
    }
    if (occupancy_system.name == column) {
        occupancy_system.visible = visible;
        return true;
    }
    if (wait_time.name == column) {
        wait_time.visible = visible;
        return true;
    }
    if (wait_time_var.name == column) {
        wait_time_var.visible = visible;
        return true;
    }
    if (throughput.name == column) {
        throughput.visible = visible;
        return true;
    }
    if (resp_time.name == column) {
        resp_time.visible = visible;
        return true;
    }
    if (resp_time_var.name == column) {
        resp_time_var.visible = visible;
        return true;
    }
    if (preemption_avg.name == column) {
        preemption_avg.visible = visible;
        return true;
    }
    if (warnings.name == column) {
        warnings.visible = visible;
        return true;
    }
    return false;
}
