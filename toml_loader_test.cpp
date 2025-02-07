//
// Created by Marco Ciotola on 26/01/25.
//

#include <iostream>
#include <map>
#include <mjqm-settings/toml_overrides.h>
#include <mjqm-settings/toml_loader.h>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    std::string_view filename(argv[1]);
    std::cout << "Reading TOML file: " << filename << std::endl;

    std::map<std::string, std::vector<std::string>> map = {
        {"simulation.smash.window", {"6", "2", "3", "4", "5"}}
    };

    toml::table table = toml::parse_file(filename);

    std::cout << "Table: " << table << std::endl;
    std::cout << "Variations: " << table.at_path("variation") << std::endl;
    std::cout << "Variations: " << table.at_path("variation").is_array() << std::endl;
    std::cout << "Variations: " << table.at_path("variation").is_array_of_tables() << std::endl;
    if (auto vars = table.at_path("variation").as_array())
    for (auto& var : *vars) {
        std::cout << "Variation: " << *var.as_table() << std::endl;
        parse_overrides_from_variation(*var.as_table());
    }
}
