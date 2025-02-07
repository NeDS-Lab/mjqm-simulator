//
// Created by Marco Ciotola on 05/02/25.
//

#define TOML_IMPLEMENTATION
#include <mjqm-settings/toml_utils.h>

template <typename VALUE_TYPE>
void overwrite_value(toml::table& data, const toml::path& path, const VALUE_TYPE& value) {
    auto parent_node = data.at_path(path.parent()).as_table();
    parent_node->insert_or_assign(path.leaf().str(), value);
}
template void overwrite_value(toml::table&, const toml::path&, const double&);
template void overwrite_value(toml::table&, const toml::path&, const long&);
template void overwrite_value(toml::table&, const toml::path&, const bool&);

template <>
void overwrite_value<std::string>(toml::table& data, const toml::path& path, const std::string& value) {
    auto parent_node = data.at_path(path.parent()).as_table();
    auto current_node = data.at_path(path);
    if (!current_node) {
        parent_node->insert_or_assign(path.leaf().str(), value);
    } else if (current_node.is_boolean()) {
        parent_node->insert_or_assign(path.leaf().str(), value == "true");
    } else if (current_node.is_integer()) {
        parent_node->insert_or_assign(path.leaf().str(), std::stol(value));
    } else if (current_node.is_floating_point()) {
        parent_node->insert_or_assign(path.leaf().str(), std::stod(value));
    } else if (current_node.is_string()) {
        parent_node->insert_or_assign(path.leaf().str(), value);
    } else {
        print_error("Unsupported type for key " << path);
    }
}

template <typename VALUE_TYPE>
void overwrite_value(toml::table& data, const std::string_view& key, const VALUE_TYPE& value) {
    toml::path path(key);
    overwrite_value<VALUE_TYPE>(data, path, value);
}
template void overwrite_value(toml::table&, const std::string_view&, const std::string&);
template void overwrite_value(toml::table&, const std::string_view&, const double&);
template void overwrite_value(toml::table&, const std::string_view&, const long&);
template void overwrite_value(toml::table&, const std::string_view&, const bool&);
template <>
void overwrite_value(toml::table& data, const std::string_view& key, const std::string_view& value) {
    overwrite_value(data, key, std::string(value));
}
