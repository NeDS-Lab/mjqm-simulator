# if math is a library, use add_library() instead
add_library(math
        confidence_intervals.cpp
        confidence_intervals.h
        sampler.h
        samplers.h
        samplers/bounded_pareto.hpp
        samplers/deterministic.hpp
        samplers/exponential.hpp
        samplers/frechet.hpp
        samplers/pareto.hpp
        samplers/uniform.hpp
)

#set(Boost_DEBUG ON)
#find_package(Boost COMPONENTS ALL)
#set(Boost_DEBUG OFF)

set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(math ${Boost_LIBRARIES})

#if(EXAMPLE_PROJECT_BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
