//
// Created by Marco Ciotola on 04/02/25.
//

#ifndef TOML_UTILS_H
#define TOML_UTILS_H

#ifndef TOML_ENABLE_UNRELEASED_FEATURES
#define TOML_ENABLE_UNRELEASED_FEATURES 1
#include "toml++/toml.h"
#endif

#include <iostream>
#include <optional>
#include <string>

#define RESET "\033[0m"
#define BLACK "\033[30m" // Black
#define RED "\033[31m" // Red
#define GREEN "\033[32m" // Green
#define YELLOW "\033[33m" // Yellow
#define BLUE "\033[34m" // Blue
#define MAGENTA "\033[35m" // Magenta
#define CYAN "\033[36m" // Cyan
#define WHITE "\033[37m" // White
#define BOLDBLACK "\033[1m\033[30m" // Bold Black
#define BOLDRED "\033[1m\033[31m" // Bold Red
#define BOLDGREEN "\033[1m\033[32m" // Bold Green
#define BOLDYELLOW "\033[1m\033[33m" // Bold Yellow
#define BOLDBLUE "\033[1m\033[34m" // Bold Blue
#define BOLDMAGENTA "\033[1m\033[35m" // Bold Magenta
#define BOLDCYAN "\033[1m\033[36m" // Bold Cyan
#define BOLDWHITE "\033[1m\033[37m" // Bold White

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
    print_error("Value missing in TOML file " << path);
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
