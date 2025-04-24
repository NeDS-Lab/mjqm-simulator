//
// Created by Marco Ciotola on 05/02/25.
//

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <mjqm-settings/toml_overrides.h>
#include <mjqm-settings/toml_utils.h>

std::vector<std::multimap<std::string, ConfigValue>> parse_overrides_from_args(int argc, char* argv[], int start_from) {
    // accept any type of value:
    // --arrival.rate 0.1 0.2 0.3
    // --policy "most server first" smash
    // and allow to group them by separating them with --pivot
    // --pivot --arrival.rate 0.1 0.2 0.3 --policy smash --pivot --arrival.rate 0.2 0.3 0.4 --policy "server filling"
    std::vector<std::multimap<std::string, ConfigValue>> overrides;
    overrides.emplace_back();
    int i = start_from;
    while (i < argc) {
        std::string arg(argv[i]);
        if (arg == "--pivot") {
            if (!overrides.back().empty()) {
                overrides.emplace_back();
            }
            ++i;
        } else if (arg.starts_with("--") && arg.length() > 2) {
            std::string key = arg.substr(2);
            ++i;
            while (i < argc && !std::string(argv[i]).starts_with("--")) {
                overrides.back().emplace(key, argv[i]);
                ++i;
            }
        } else {
            std::cerr << "Invalid argument: " << arg << std::endl;
            ++i;
        }
    }
    if (overrides.back().empty()) {
        overrides.pop_back();
    }
    // print them out
    // for (const auto& pivot : overrides) {
    //     std::cout << "Pivot" << std::endl;
    //     for (const auto& val : pivot) {
    //         std::cout << val.first << ": " << val.second << " " << std::endl;
    //     }
    //     std::cout << std::endl;
    // }
    return overrides;
}

std::multimap<std::string, ConfigValue> parse_overrides_from_pivot(const toml::table& table) {
    // accept any type of value:
    // [[pivot]]
    // arrival.rate = [ 0.1 0.2 0.3 ]
    // policy = [ "most server first", "smash" ]
    // policy = "smash" # just one value for overriding without matrix effect
    std::multimap<std::string, ConfigValue> overrides;
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
                subvalue.visit([&](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (is_override_value<T>) {
                        overrides.emplace(subpath, val.get());
                    } else if constexpr (toml::is_table<T>) {
                        new_tables.emplace(subpath, val);
                    } else if constexpr (toml::is_array<T>) {
                        val.for_each([&](auto&& val) {
                            using T = std::decay_t<decltype(val)>;
                            if constexpr (is_override_value<T>) {
                                overrides.emplace(subpath, val.get());
                            } else if constexpr (toml::is_table<T>) {
                                overrides.emplace(subpath, val);
                            } else {
                                print_error("Unsupported type for one element of pivot key " << subpath);
                            }
                        });
                    } else {
                        print_error("Unsupported type for pivot key " << subpath);
                    }
                });
            }
        }
        tables = std::move(new_tables);
    } while (!tables.empty());
    // print them out
    // auto curr = overrides.begin();
    // while (curr != overrides.end()) {
    //     const auto& key = curr->first;
    //     std::cout << key << ": ";
    //     for (auto next = overrides.upper_bound(key); curr != next; ++curr) {
    //         std::cout << curr->second << " ";
    //     }
    //     std::cout << std::endl;
    // }
    return overrides;
}

std::set<std::string> keys(const std::multimap<std::string, ConfigValue>& overrides) {
    std::set<std::string> keys;
    for (const auto& [key, value] : overrides) {
        keys.emplace(key);
    }
    return keys;
}

std::multimap<std::string, ConfigValue>
merge_overrides(const std::multimap<std::string, ConfigValue>& base,
                const std::multimap<std::string, ConfigValue>& higher_priority) {
    std::multimap merged(base);
    for (const auto& key : keys(higher_priority)) {
        merged.erase(key);
        auto [start, end] = higher_priority.equal_range(key);
        for (; start != end; ++start) {
            merged.emplace(key, start->second);
        }
    }
    return merged;
}

toml_overrides::toml_overrides(const std::multimap<std::string, ConfigValue>& overrides) : overrides(0) {
    for (const auto& key : keys(overrides)) {
        auto& o_pairs = this->overrides.emplace_back();
        auto [start, end] = overrides.equal_range(key);
        for (; start != end; ++start) {
            o_pairs.emplace_back(key, start->second);
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

std::ostream& operator<<(std::ostream& os, const ConfigValue& variant) {
    std::visit([&](auto&& value) { os << value; }, variant);
    return os;
}
