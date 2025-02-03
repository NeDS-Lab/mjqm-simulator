cmake_minimum_required(VERSION 3.28)
#cmake_policy(SET CMP0167 NEW)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
add_link_options($<$<CONFIG:RELEASE>:-s>)
add_compile_options($<$<CONFIG:RELEASE>:-Ofast>)
add_compile_options($<$<CONFIG:RELEASE>:-march=native>)

link_libraries("-static")

project(mjqm-simulator
        VERSION 0.1.0
        DESCRIPTION "Simulator for Multiserver Job Queuing Model (MJQM)"
        HOMEPAGE_URL "https://github.com/NeDS-Lab/mjqm-simulator"
        LANGUAGES CXX
)

include_directories(
        libs/math/include
        libs/policy/include
        libs/simulator/include
)

add_subdirectory(libs/math)
add_subdirectory(libs/policy)
add_subdirectory(libs/simulator)

add_executable(main_allPols main_allPols.cpp)
target_link_libraries(main_allPols)

add_executable(simulator_toc simulator_toc.cpp)
target_link_libraries(simulator_toc)

add_executable(simulator_smash simulator_smash.cpp)
target_link_libraries(simulator_smash math policies stats)

add_executable(simulator_toml simulator_toml.cpp)
target_link_libraries(simulator_toml math policies stats toml_loader)

add_executable(toml_loader_test toml_loader_test.cpp)
target_link_libraries(toml_loader_test toml_loader)

foreach (target main_allPols simulator_toc simulator_smash simulator_toml toml_loader_test)
    add_custom_command(
            TARGET ${target} POST_BUILD
            BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/${target}
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/${target}
            ${CMAKE_CURRENT_SOURCE_DIR}/${target})
endforeach ()

add_custom_target(test_all)

function(add_custom_test_ff)
    cmake_parse_arguments(
            T # prefix of output variables
            "" # list of names of the boolean arguments (only defined ones will be true)
            "TARGET;NAME;FILE" # list of names of mono-valued arguments
            "ARGS" # list of names of multi-valued arguments (output variables are lists)
            ${ARGN} # arguments of the function to parse, here we take the all original ones
    )
    add_custom_command(
            OUTPUT Results/${T_TARGET}/${T_FILE}
            BYPRODUCTS Results/${T_TARGET}/${T_FILE}.out.txt Results/${T_TARGET}/${T_FILE}.diff.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND mkdir -p Results/${T_TARGET}
            COMMAND ${T_TARGET} ${T_ARGS} > Results/${T_TARGET}/${T_FILE}.out.txt 2>&1
            COMMAND python3 ensure_same_results.py Results/${T_TARGET}/${T_FILE} test/expected/${T_FILE} > Results/${T_TARGET}/${T_FILE}.diff.txt 2>&1
            COMMAND grep -q "Data is the same" Results/${T_TARGET}/${T_FILE}.diff.txt && cmake -E cmake_echo_color --green "${T_TARGET} PASSED test ${T_FILE}" || cmake -E cmake_echo_color --red "${T_TARGET} PASSED test ${T_FILE}" VERBATIM
    )
    add_custom_target(test_${T_TARGET}_${T_NAME} DEPENDS Results/${T_TARGET}/${T_FILE})
    add_dependencies(test_${T_TARGET} test_${T_TARGET}_${T_NAME})
endfunction(add_custom_test_ff)

foreach (target main_allPols simulator_toc simulator_smash)
    add_custom_target(test_${target})
    add_dependencies(test_all test_${target})

    add_custom_test_ff(
            TARGET ${target}
            NAME oneOrAll-test1
            FILE overLambdas-nClasses2-N50-Win1-Exponential-oneOrAll-test1.csv
            ARGS oneOrAll-test1 50 1 exp 100000 10
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_MostServerFirst_exp
            FILE overLambdas-nClasses4-N16-Win0-Exponential-testing_4C_16.csv
            ARGS testing_4C_16 16 0 exp 100000 20
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_smash1_exp
            FILE overLambdas-nClasses4-N16-Win1-Exponential-testing_4C_16.csv
            ARGS testing_4C_16 16 1 exp 100000 20
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_smash4_exp
            FILE overLambdas-nClasses4-N16-Win4-Exponential-testing_4C_16.csv
            ARGS testing_4C_16 16 4 exp 100000 20
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_ServerFillingMem_exp
            FILE overLambdas-nClasses4-N16-Win-2-Exponential-testing_4C_16.csv
            ARGS testing_4C_16 16 -2 exp 100000 20
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_BackFilling_exp
            FILE overLambdas-nClasses4-N16-Win-3-Exponential-testing_4C_16.csv
            ARGS testing_4C_16 16 -3 exp 100000 20
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_smash1_fre
            FILE overLambdas-nClasses4-N16-Win1-Frechet-testing_4C_16.csv
            ARGS testing_4C_16 16 1 fre 100000 20
    )
    add_custom_test_ff(
            TARGET ${target}
            NAME testing_4C_16_smash1_uni
            FILE overLambdas-nClasses4-N16-Win1-Uniform-testing_4C_16.csv
            ARGS testing_4C_16 16 1 uni 100000 20
    )
endforeach ()
