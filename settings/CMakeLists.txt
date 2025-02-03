set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        c4369ae1d8955cae20c4ab40b9813ef4b60e48be
)
FetchContent_MakeAvailable(tomlplusplus)

add_library(toml_loader
        toml_loader.cpp
        toml_loader.h
)
target_link_libraries(toml_loader tomlplusplus::tomlplusplus math policies)
