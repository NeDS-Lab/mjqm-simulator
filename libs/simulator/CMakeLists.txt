add_library(stats
        src/mjqm-simulator/experiment_stats.cpp
)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        c4369ae1d8955cae20c4ab40b9813ef4b60e48be
)
FetchContent_MakeAvailable(tomlplusplus)

add_library(toml_loader
        src/mjqm-settings/toml_utils.cpp
        src/mjqm-settings/toml_overrides.cpp
        src/mjqm-settings/toml_loader.cpp
        src/mjqm-settings/toml_policies_loaders.cpp
        src/mjqm-settings/toml_distributions_loaders.cpp
)
target_link_libraries(toml_loader tomlplusplus::tomlplusplus math policies stats)
