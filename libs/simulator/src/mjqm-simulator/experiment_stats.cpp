//
// Created by Marco Ciotola on 23/01/25.
//

#include <iostream>
#include <vector>

#include <mjqm-simulator/experiment_stats.h>

#include "mjqm-math/confidence_intervals.h"

// ****** Stat
// *** visitor
VariantStat Stat::visit_value(const std::function<Confidence_inter(Confidence_inter const&)>& confidence_inter,
                              const std::function<bool(bool const&)>& _bool,
                              const std::function<double(double const&)>& _double,
                              const std::function<long(long const&)>& _long,
                              const std::function<std::string(std::string const&)>& _string) const {
    return std::visit(
        [&](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Confidence_inter>) {
                return VariantStat(confidence_inter(value));
            } else if constexpr (std::is_same_v<T, bool>) {
                return VariantStat(_bool(value));
            } else if constexpr (std::is_same_v<T, double>) {
                return VariantStat(_double(value));
            } else if constexpr (std::is_same_v<T, long>) {
                return VariantStat(_long(value));
            } else if constexpr (std::is_same_v<T, std::string>) {
                return VariantStat(_string(value));
            } else {
                return VariantStat("N/A");
            }
        },
        this->value);
}

// *** outputs
void Stat::add_headers(std::vector<std::string>& headers) const {
    if (!visible) {
        return;
    }
    std::string header = name;
    if (!prefix.empty()) {
        header = prefix + " " + header;
    }
    headers.emplace_back(header);
    if (has_confidence_interval) {
        headers.emplace_back(header + " ConfInt");
    }
}

std::ostream& operator<<(std::ostream& os, const Stat& m) {
    if (!m.visible) {
        return os;
    }
    m.visit_value(
        [&os](const Confidence_inter& value) {
            os << value;
            return value;
        },
        [&os](const bool& value) {
            os << value;
            return value;
        },
        [&os](const double& value) {
            os << value;
            return value;
        },
        [&os](const long& value) {
            os << value;
            return value;
        },
        [&os](const std::string& value) {
            os << value;
            return value;
        });
    return os;
}

// *** operators
VariantStat Stat::operator+(Stat const& that) const {
    return visit_value([&that](Confidence_inter const& c) { return c + std::get<Confidence_inter>(that.value); },
                       [&that](bool const& b) { return b || std::get<bool>(that.value); },
                       [&that](double const& d) { return d + std::get<double>(that.value); },
                       [&that](long const& l) { return l + std::get<long>(that.value); },
                       [&that](std::string const& s) { return s + std::get<std::string>(that.value); });
}
Stat& Stat::operator=(VariantStat const& value) {
    this->value = value;
    return *this;
}
// ****** end Stat

// ****** ClassStats
// *** outputs
void ClassStats::add_headers(std::vector<std::string>& headers) const {
    visit_stats([&headers](const Stat& s) { s.add_headers(headers); });
}
std::ostream& operator<<(std::ostream& os, ClassStats const& m) {
    m.visit_stats([&os](const Stat& s) { os << s; });
    return os;
}

// *** visibility setters
void ClassStats::set_computed_columns_visibility(bool visible) {
    edit_stats([&](Stat& s) { s.visible = visible; });
}
void ClassStats::set_column_visibility(const std::string& column, bool visible) {
    edit_stats([&](Stat& s) {
        if (s.name == column) {
            s.visible = visible;
        }
    });
}
// ****** end ClassStats

// ****** ExperimentStats
// *** visitors
void ExperimentStats::edit_all_stats(const std::function<void(Stat&)>& editor) {
    for (auto& pv : pivot_values) {
        editor(pv);
    }
    for (auto& cv : custom_values) {
        editor(cv);
    }
    for (auto& cs : class_stats) {
        cs.edit_stats(editor);
    }
    edit_computed_stats(editor);
}
void ExperimentStats::visit_all_stats(const std::function<void(const Stat&)>& visitor) const {
    for (auto& pv : pivot_values) {
        visitor(pv);
    }
    for (auto& cv : custom_values) {
        visitor(cv);
    }
    for (auto& cs : class_stats) {
        cs.visit_stats(visitor);
    }
    visit_computed_stats(visitor);
}

// *** outputs
void ExperimentStats::add_headers(std::vector<std::string>& headers, std::vector<unsigned int>&) const {
    add_headers(headers);
}
void ExperimentStats::add_headers(std::vector<std::string>& headers) const {
    visit_all_stats([&headers](const Stat& s) { s.add_headers(headers); });
}

std::ostream& operator<<(std::ostream& os, const ExperimentStats& m) {
    m.visit_all_stats([&os](const Stat& s) { os << s; });
    return os;
}

// *** visibility setters
void ExperimentStats::set_computed_columns_visibility(bool visible) {
    for (auto& cls : class_stats) {
        cls.set_computed_columns_visibility(visible);
    }
    edit_computed_stats([visible](Stat& s) { s.visible = visible; });
}
void ExperimentStats::set_column_visibility(const std::string& column, bool visible) {
    for (auto& pv : pivot_values) {
        if (column == "pivots") {
            pv.visible = visible;
        } else if (pv.name == column) {
            pv.visible = visible;
            return;
        }
    }
    if (column == "pivots") {
        return;
    }
    for (auto& cv : custom_values) {
        if (cv.name == column) {
            cv.visible = visible;
            return;
        }
    }
    edit_computed_stats([&](Stat& s) {
        if (s.name == column) {
            s.visible = visible;
        }
    });
}
void ExperimentStats::set_class_column_visibility(const std::string& column, bool visible, const std::string& cls) {
    for (auto& class_stat : class_stats) {
        if (class_stat.name == cls || cls == "*") {
            class_stat.set_column_visibility(column, visible);
            return;
        }
    }
}
// ****** end ExperimentStats
