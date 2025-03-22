//
// Created by Marco Ciotola on 23/01/25.
//

#ifndef EXPERIMENT_STATS_H
#define EXPERIMENT_STATS_H

#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <mjqm-math/confidence_intervals.h>

typedef std::variant<Confidence_inter, bool, double, long, std::string> VariantStat;

class Stat {
public:
    std::string prefix;
    const std::string name;
    const bool has_confidence_interval;
    VariantStat value;
    bool visible = true;

    Stat(std::string name, bool has_confidence_interval) :
        name{std::move(name)}, has_confidence_interval{has_confidence_interval} {}

    VariantStat visit_value(const std::function<Confidence_inter(Confidence_inter const&)>& confidence_inter,
                            const std::function<bool(bool const&)>& _bool,
                            const std::function<double(double const&)>& _double,
                            const std::function<long(long const&)>& _long,
                            const std::function<std::string(std::string const&)>& _string) const;

    void add_headers(std::vector<std::string>& headers) const;
    VariantStat operator+(Stat const& that) const;
    Stat& operator=(VariantStat const& value);
};
std::ostream& operator<<(std::ostream& os, const Stat& m);

class ClassStats {
public:
    const std::string name;
    Stat occupancy_buf{"Queue", true};
    Stat occupancy_ser{"Service", true};
    Stat occupancy_system{"System", true};
    Stat wait_time{"Waiting", true};
    Stat wait_time_var{"Waiting Variance", true};
    Stat throughput{"Throughput", true};
    Stat resp_time{"RespTime", true};
    Stat resp_time_var{"RespTime Variance", true};
    Stat preemption_avg{"Preemption", true};
    Stat warnings{"Stability Check", false};
    Stat arrival_rate{"Arrival Rate", false};

    explicit ClassStats(const std::string& name) : name{name} {
        edit_stats([name](Stat& s) { s.prefix = "T" + name; });
    }

    // visitors
    void edit_stats(const std::function<void(Stat&)>& editor) {
        editor(occupancy_buf);
        editor(occupancy_ser);
        editor(occupancy_system);
        editor(wait_time);
        editor(wait_time_var);
        editor(throughput);
        editor(resp_time);
        editor(resp_time_var);
        editor(preemption_avg);
        editor(warnings);
        editor(arrival_rate);
    }

    void visit_stats(const std::function<void(const Stat&)>& visitor) const {
        visitor(occupancy_buf);
        visitor(occupancy_ser);
        visitor(occupancy_system);
        visitor(wait_time);
        visitor(wait_time_var);
        visitor(throughput);
        visitor(resp_time);
        visitor(resp_time_var);
        visitor(preemption_avg);
        visitor(warnings);
        visitor(arrival_rate);
    }

    // outputs
    void add_headers(std::vector<std::string>& headers) const;
    friend std::ostream& operator<<(std::ostream& os, ClassStats const& m);

    // visibility setters
    void set_computed_columns_visibility(bool visible);
    void set_column_visibility(const std::string& column, bool visible);
};

class ExperimentStats {
    std::vector<Stat> pivot_values{};
    std::vector<Stat> custom_values{};

public:
    std::vector<ClassStats> class_stats{};
    Stat wasted{"Wasted Servers", true};
    Stat utilisation{"Utilisation", true};
    Stat occupancy_tot{"Queue Total", true};
    Stat wait_tot{"WaitTime Total", true};
    Stat wait_var_tot{"WaitTime Variance", true};
    Stat resp_tot{"RespTime Total", true};
    Stat resp_var_tot{"RespTime Variance", true};
    Stat window_size{"Window Size", true};
    Stat violations{"FIFO Violations", true};
    Stat timings_tot{"Run Duration", true};
    Stat big_seq_avg_len{"Big Sequence Length", true};
    Stat small_seq_avg_len{"Small Sequence Length", true};
    Stat big_seq_avg_dur{"Big Sequence Duration", true};
    Stat small_seq_avg_dur{"Small Sequence Duration", true};
    Stat big_seq_amount{"Big Sequence Amount", true};
    Stat small_seq_amount{"Small Sequence Amount", true};
    Stat phase_two_dur{"Phase Two Duration", true};
    Stat phase_three_dur{"Phase Three Duration", true};
    Stat arrival_rate{"Arrival Rate Total", false};

    // visitors
    void edit_computed_stats(const std::function<void(Stat&)>& editor) {
        editor(wasted);
        editor(utilisation);
        editor(occupancy_tot);
        editor(wait_tot);
        editor(wait_var_tot);
        editor(resp_tot);
        editor(resp_var_tot);
        editor(window_size);
        editor(violations);
        editor(timings_tot);
        editor(big_seq_avg_len);
        editor(small_seq_avg_len);
        editor(big_seq_avg_dur);
        editor(small_seq_avg_dur);
        editor(big_seq_amount);
        editor(small_seq_amount);
        editor(phase_two_dur);
        editor(phase_three_dur);
        editor(arrival_rate);
    }

    void visit_computed_stats(const std::function<void(const Stat&)>& visitor) const {
        visitor(wasted);
        visitor(utilisation);
        visitor(occupancy_tot);
        visitor(wait_tot);
        visitor(wait_var_tot);
        visitor(resp_tot);
        visitor(resp_var_tot);
        visitor(window_size);
        visitor(violations);
        visitor(timings_tot);
        visitor(big_seq_avg_len);
        visitor(small_seq_avg_len);
        visitor(big_seq_avg_dur);
        visitor(small_seq_avg_dur);
        visitor(big_seq_amount);
        visitor(small_seq_amount);
        visitor(phase_two_dur);
        visitor(phase_three_dur);
        visitor(arrival_rate);
    }
    void edit_all_stats(const std::function<void(Stat&)>& editor);
    void visit_all_stats(const std::function<void(const Stat&)>& visitor) const;

    // outputs
    friend std::ostream& operator<<(std::ostream& os, ExperimentStats const& m);
    void add_headers(std::vector<std::string>& headers, std::vector<unsigned int>& sizes) const;
    void add_headers(std::vector<std::string>& headers) const;

    // add elements
    ClassStats& add_class(const std::string& name) { return std::ref(class_stats.emplace_back(name)); }

    template <typename VAL_TYPE>
    bool add_pivot_column(const std::string& name, const VAL_TYPE& value) {
        pivot_values.emplace_back(name, false);
        pivot_values.back() = value;
        return true;
    }

    template <typename VAL_TYPE>
    bool add_custom_column(const std::string& name, const VAL_TYPE& value) {
        custom_values.emplace_back(name, false);
        custom_values.back() = value;
        return true;
    }

    // visibility setters
    void set_computed_columns_visibility(bool visible);

    void set_column_visibility(const std::string& column) {
        if (column.starts_with("-"))
            return set_column_visibility(column.substr(1), false);
        return set_column_visibility(column, true);
    }
    void set_column_visibility(const std::string& column, bool visible);

    void set_class_column_visibility(const std::string& column, const std::string& cls = "*") {
        if (column.starts_with("-"))
            return set_class_column_visibility(column.substr(1), false, cls);
        return set_class_column_visibility(column, true, cls);
    }
    void set_class_column_visibility(const std::string& column, bool visible, const std::string& cls = "*");
};

#endif // EXPERIMENT_STATS_H
