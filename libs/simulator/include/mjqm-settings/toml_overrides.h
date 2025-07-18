//
// Created by Marco Ciotola on 05/02/25.
//

#ifndef TOML_OVERRIDES_H
#define TOML_OVERRIDES_H

#include <map>
#include <string>
#include <variant>
#include <vector>

#include <mjqm-settings/toml_utils.h>

typedef std::variant<bool, double, long, std::string, toml::table> ConfigValue;
std::ostream& operator<<(std::ostream& os, const ConfigValue& variant);

template <typename T>
constexpr bool is_override_value = (toml::is_string<T> || toml::is_number<T> || toml::is_boolean<T>);

std::vector<std::multimap<std::string, ConfigValue>> parse_overrides_from_args(int argc, char* argv[],
                                                                               int start_from = 2);
std::multimap<std::string, ConfigValue> parse_overrides_from_pivot(const toml::table& table);
std::multimap<std::string, ConfigValue> merge_overrides(const std::multimap<std::string, ConfigValue>& base,
                                                        const std::multimap<std::string, ConfigValue>& higher_priority);

class toml_overrides {
    std::vector<std::vector<std::pair<std::string, ConfigValue>>> overrides;

public:
    explicit toml_overrides(const std::multimap<std::string, ConfigValue>& overrides);

    size_t size() const {
        size_t s = 1;
        for (const auto& o : overrides) {
            s *= o.size();
        }
        return s;
    }

    class iterator {
        using self_type = iterator;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = std::vector<std::pair<std::string, ConfigValue>>;

        std::vector<size_t> state;
        const std::vector<std::vector<std::pair<std::string, ConfigValue>>> data;

    public:
        explicit iterator(const toml_overrides& data) : state(data.overrides.size() + 1, 0), data(data.overrides) {}

        value_type operator*() const {
            value_type result(0);
            result.reserve(data.size());
            for (size_t i = 0; i < data.size(); ++i) {
                result.emplace_back(data[i][state[i]]);
            }
            return result;
        }

        iterator operator++() { return *this + 1; }
        iterator operator++(int) {
            iterator tmp(*this);
            operator++();
            return tmp;
        }
        iterator operator+(size_t n) {
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
        bool operator==(const iterator& other) const { return state == other.state; }
        bool operator!=(const iterator& other) const { return state != other.state; }
    };

    iterator begin() const { return iterator{*this}; }
    iterator end() const { return iterator{*this} + size(); }
};

#endif // TOML_OVERRIDES_H
