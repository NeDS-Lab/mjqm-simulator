//
// Created by Marco Ciotola on 05/02/25.
//

#include <map>
#include <mjqm-settings/toml_overrides.h>
#include <string>
#include <string_view>
#include <vector>

toml_overrides::toml_overrides(const std::map<std::string, std::vector<std::string>>& overrides) : overrides(0) {
    for (const auto& [key, values] : overrides) {
        auto o_pairs = this->overrides.emplace_back(values.size());
        for (auto& value : values) {
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
toml_overrides::iterator::iterator(const toml_overrides& data) :
    pairs(data.overrides.size()), state(data.overrides.size() + 1, 0), data(data.overrides) {}
toml_overrides::iterator::value_type toml_overrides::iterator::operator*() const {
    value_type result(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result.push_back(data[i][state[i]]);
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
