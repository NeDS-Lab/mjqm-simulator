//
// Created by Marco Ciotola on 26/01/25.
//

#include <iostream>
#include <mjqm-settings/toml_overrides.h>
#include <mjqm-settings/toml_loader.h>
#include <string>

int main(int argc, char* argv[])
{
    std::string filename(argv[1]);
    std::cout << "Reading TOML file: " << filename << std::endl;

    auto overrides = parse_overrides_from_args(argc, argv);
    auto experiments = from_toml("Inputs/" + filename + ".toml", overrides);

    for (const auto& [success, config] : *experiments) {
        if (!success) {
            std::cerr << "Error reading a variation" << std::endl << std::endl;
            continue;
        }
        std::cout << config << std::endl;
        std::cout << config.toml << std::endl << std::endl;
    }
}
