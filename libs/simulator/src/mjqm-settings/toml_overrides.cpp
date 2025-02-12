//
// Created by Marco Ciotola on 05/02/25.
//

#include <iostream>
#include <map>
#include <mjqm-settings/toml_overrides.h>
#include <mjqm-settings/toml_utils.h>
#include <string>
#include <vector>

std::map<std::string, std::vector<std::string>> parse_overrides_from_args(int argc, char* argv[], int start_from) {
    // accept any type of value:
    // --arrival.rate 0.1 0.2 0.3
    // --policy "most server first" smash
    std::map<std::string, std::vector<std::string>> overrides;
    int i = start_from;
    while (i < argc) {
        std::string arg(argv[i]);
        if (arg.starts_with("--") && arg.length() > 2) {
            std::string key = arg.substr(2);
            overrides[key] = {};
            i++;
            while (i < argc && argv[i][0] != '-') {
                overrides[key].emplace_back(argv[i]);
                i++;
            }
            if (overrides[key].empty()) {
                std::cerr << "Missing values for argument: " << arg << std::endl;
                overrides.erase(key);
            }
        } else {
            std::cerr << "Invalid argument: " << arg << std::endl;
            i++;
        }
    }
    // print them out
    // for (const auto& [key, values] : overrides) {
    //     std::cout << key << ": ";
    //     for (const auto& val : values) {
    //         std::cout << val << " ";
    //     }
    //     std::cout << std::endl;
    // }
    return overrides;
}

std::string as_string(const toml::node& node) {
    switch (node.type()) {
    case toml::node_type::string:
        return node.value<std::string>().value();
    case toml::node_type::integer:
        return std::to_string(node.value<int64_t>().value());
    case toml::node_type::floating_point:
        return std::to_string(node.value<double>().value());
    case toml::node_type::boolean:
        return node.value<bool>().value() ? "true" : "false";
    default:
        throw std::runtime_error("Unsupported value type");
    }
}

std::map<std::string, std::vector<std::string>> parse_overrides_from_variation(const toml::table& table) {
    // accept any type of value:
    // [[variation]]
    // arrival.rate = [ 0.1 0.2 0.3 ]
    // policy = [ "most server first" "smash" ]
    // policy = "smash" # just one value for overriding without matrix effect
    std::map<std::string, std::vector<std::string>> overrides;
    std::map<std::string, toml::table> tables{{"", table}};
    do {
        std::map<std::string, toml::table> new_tables{};
        for (auto& [key, value] : tables) {
            toml::path suppath;
            if (key == "") {
                suppath = toml::path();
            } else {
                suppath = toml::path(key);
            }
            for (const auto& [subkey, subvalue] : value) {
                auto subpath = std::string(suppath + subkey);
                if (subvalue.is_array()) {
                    auto& vals = overrides[subpath];
                    for (auto&& val : *subvalue.as_array()) {
                        vals.emplace_back(as_string(val));
                    }
                } else if (subvalue.is_table()) {
                    new_tables[subpath] = *subvalue.as_table();
                } else {
                    overrides[subpath] = {as_string(subvalue)};
                }
            }
        }
        tables = std::move(new_tables);
    } while (!tables.empty());
    // print them out
    // for (const auto& [key, values] : overrides) {
    //     std::cout << key << ": ";
    //     for (const auto& val : values) {
    //         std::cout << val << " ";
    //     }
    //     std::cout << std::endl;
    // }
    return overrides;
}

std::map<std::string, std::vector<std::string>>
merge_overrides(const std::map<std::string, std::vector<std::string>>& base,
                const std::map<std::string, std::vector<std::string>>& higher_priority) {
    std::map merged(base);
    for (const auto& [key, values] : higher_priority) {
        if (merged.contains(key)) {
            merged.erase(key);
        }
        merged.emplace(key, values);
    }
    return merged;
}

toml_overrides::toml_overrides(const std::map<std::string, std::vector<std::string>>& overrides) : overrides(0) {
    for (const auto& [key, values] : overrides) {
        auto& o_pairs = this->overrides.emplace_back();
        o_pairs.reserve(values.size());
        for (const auto& value : values) {
            o_pairs.emplace_back(key, value);
        }
    }
}
size_t toml_overrides::size() const {
    size_t s = 1;
    for (const auto& o : overrides) {
        s *= o.size();
    }
    return s;
}
toml_overrides::iterator::value_type toml_overrides::iterator::operator*() const {
    value_type result(0);
    result.reserve(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result.emplace_back(data[i][state[i]]);
    }
    return result;
}
toml_overrides::iterator toml_overrides::iterator::operator++() { return *this + 1; }
toml_overrides::iterator toml_overrides::iterator::operator++(int) {
    iterator tmp(*this);
    operator++();
    return tmp;
}
toml_overrides::iterator toml_overrides::iterator::operator+(size_t n) {
    for (size_t i = 0; i < data.size() && n > 0; ++i) {
        n += state[i];
        state[i] = n % data[i].size();
        n /= data[i].size();
    }
    if (n > 0) {
        state[data.size()] = 1;
    }
    return *this;
}
bool toml_overrides::iterator::operator==(const iterator& other) const { return state == other.state; }
bool toml_overrides::iterator::operator!=(const iterator& other) const { return state != other.state; }
toml_overrides::iterator toml_overrides::begin() const { return iterator{*this}; }
toml_overrides::iterator toml_overrides::end() const { return iterator{*this} + size(); }
