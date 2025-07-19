//
// Created by Marco Ciotola on 26/01/25.
//

#include <iostream>
#include <string>

#include <mjqm-settings/toml_loader.h>
#include <mjqm-settings/toml_overrides.h>

int main(int argc, char* argv[]) {
    std::string filename(argv[1]);
    if (!filename.ends_with(".toml")) {
        filename += ".toml";
    }
    std::cout << "Reading TOML file: " << filename << std::endl;

    auto overrides = parse_overrides_from_args(argc, argv);

    fs::path input_file = fs::current_path() / "Inputs" / filename;
    auto experiments = from_toml(input_file, overrides);

    for (const auto& [success, config] : *experiments) {
        if (!success) {
            std::cerr << "Error reading a pivot" << std::endl << std::endl;
            continue;
        }
        std::cout << config << std::endl;
        std::cout << config.toml << std::endl << std::endl;
    }
}
