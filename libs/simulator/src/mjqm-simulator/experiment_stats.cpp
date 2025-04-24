//
// Created by Marco Ciotola on 23/01/25.
//

#include <iostream>
#include <vector>

#include <mjqm-simulator/experiment_stats.h>

// ****** Stat
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
    std::visit([&](auto&& value) { os << value; }, m.value);
    return os << ";";
}

// *** operators
VariantStat Stat::operator+(Stat const& that) const {
    VariantStat v = "N/A";
    std::visit(
        [&](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, bool>) {
                v = value || std::get<bool>(that.value);
            } else {
                v = value + std::get<T>(that.value);
            }
        },
        this->value);
    return v;
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
    edit_computed_stats(editor);
}
void ExperimentStats::visit_all_stats(const std::function<void(const Stat&)>& visitor) const {
    for (auto& pv : pivot_values) {
        visitor(pv);
    }
    for (auto& cv : custom_values) {
        visitor(cv);
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
