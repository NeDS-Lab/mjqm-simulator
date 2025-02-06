//
// Created by Marco Ciotola on 05/02/25.
//

#ifndef TOML_OVERRIDES_H
#define TOML_OVERRIDES_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

class toml_overrides {
    std::vector<std::vector<std::pair<std::string_view, std::string_view>>> overrides;

public:
    explicit toml_overrides(const std::map<std::string, std::vector<std::string>>& overrides);

    size_t size() const;

    class iterator {
        using self_type = iterator;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = std::vector<std::pair<std::string_view, std::string_view>>;

    public:
        const size_t pairs;

    private:
        std::vector<size_t> state;
        const std::vector<std::vector<std::pair<std::string_view, std::string_view>>>& data;

    public:
        explicit iterator(const toml_overrides& data);

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
