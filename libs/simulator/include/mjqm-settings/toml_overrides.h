//
// Created by Marco Ciotola on 05/02/25.
//

#ifndef TOML_OVERRIDES_H
#define TOML_OVERRIDES_H

#include <map>
#include <string>
#include <vector>

#include <mjqm-settings/toml_utils.h>

std::vector<std::multimap<std::string, std::string>> parse_overrides_from_args(int argc, char* argv[],
                                                                               int start_from = 2);
std::multimap<std::string, std::string> parse_overrides_from_pivot(const toml::table& table);
std::multimap<std::string, std::string> merge_overrides(const std::multimap<std::string, std::string>& base,
                                                        const std::multimap<std::string, std::string>& higher_priority);

class toml_overrides {
    std::vector<std::vector<std::pair<std::string, std::string>>> overrides;

public:
    explicit toml_overrides(const std::multimap<std::string, std::string>& overrides);

    size_t size() const;

    class iterator {
        using self_type = iterator;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = std::vector<std::pair<std::string, std::string>>;

        std::vector<size_t> state;
        const std::vector<std::vector<std::pair<std::string, std::string>>> data;

    public:
        explicit iterator(const toml_overrides& data) : state(data.overrides.size() + 1, 0), data(data.overrides) {}

        value_type operator*() const;

        iterator operator++();
        iterator operator++(int);
        iterator operator+(size_t n);
        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
    };

    iterator begin() const;
    iterator end() const;
};

#endif // TOML_OVERRIDES_H
