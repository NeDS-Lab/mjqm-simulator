add_library(math
        src/mjqm-math/confidence_intervals.cpp
        src/mjqm-math/random_ecuyer.cpp
        src/mjqm-math/random_mersenne.cpp
)

# Include the Boost library
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.83 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(math ${Boost_LIBRARIES})

# Include the L'Écuyer RngStreams library
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
include(FetchContent)
FetchContent_Declare(
        RngStreams
        GIT_REPOSITORY https://github.com/umontreal-simul/RngStreams.git
        GIT_TAG        b47f576ee28951bc437b3ed686ff4b81a96982c0
)
FetchContent_MakeAvailable(RngStreams)
target_link_libraries(math RngStreams::RngStreams)
