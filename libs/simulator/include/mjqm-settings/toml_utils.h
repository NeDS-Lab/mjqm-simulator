//
// Created by Marco Ciotola on 04/02/25.
//

#ifndef TOML_UTILS_H
#define TOML_UTILS_H

#include <iostream>
#include <optional>
#include <string_view>

#define TOML_ENABLE_UNRELEASED_FEATURES 1
#define TOML_HEADER_ONLY 0
#include "toml++/toml.h"

#define RESET "\033[0m"
#define RED "\033[31m"
#define BOLDRED "\033[1m\033[31m"

#ifndef error_highlight
#define error_highlight(a) BOLDRED << a << RESET << RED
#endif // error_highlight

#ifndef print_error
#define print_error(a) std::cerr << error_highlight("Error: ") << a << RESET << std::endl
#endif // print_error

template <typename VAR_TYPE>
bool load_into(const toml::table& data, const std::string_view path, VAR_TYPE& value) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    if (val.has_value()) {
        value = val.value();
        return true;
    }
    print_error("Value missing in TOML file " << error_highlight(path));
    return false;
}

template <typename VAR_TYPE>
bool load_into(const toml::table& data, const std::string_view path, VAR_TYPE& value, const VAR_TYPE& def) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    value = val.value_or(def);
    return true;
}

template <typename VAR_TYPE>
VAR_TYPE either(const std::optional<VAR_TYPE>& first, const std::optional<VAR_TYPE>& second) {
    return first.has_value() ? first.value() : second.value();
}

template <typename VAR_TYPE>
const std::optional<VAR_TYPE> either_optional(const std::optional<VAR_TYPE>& first,
                                              const std::optional<VAR_TYPE>& second) {
    return first.has_value() ? first : second;
}

template <typename VAR_TYPE>
const std::optional<VAR_TYPE> either_optional(const toml::node_view<const toml::node>& first,
                                              const toml::node_view<const toml::node>& second) {
    return either_optional(first.value<VAR_TYPE>(), second.value<VAR_TYPE>());
}

template <typename VAR_TYPE>
void overwrite_value(toml::table& data, const std::string_view& key, const VAR_TYPE& value);
template <typename VAR_TYPE>
void overwrite_value(toml::table& data, const toml::path& path, const VAR_TYPE& value);

#endif // TOML_UTILS_H
