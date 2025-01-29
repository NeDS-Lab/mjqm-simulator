add_library(settings
        experiment.hpp
        loader.hpp
)
set_target_properties(settings PROPERTIES LINKER_LANGUAGE CXX)

add_library(settings_toml
        toml_loader.hpp
)

include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        c4369ae1d8955cae20c4ab40b9813ef4b60e48be
)
FetchContent_MakeAvailable(tomlplusplus)
target_link_libraries(settings_toml tomlplusplus::tomlplusplus)
set_target_properties(settings_toml PROPERTIES LINKER_LANGUAGE CXX)

#if(EXAMPLE_PROJECT_BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
