add_library(math
        src/mjqm-math/confidence_intervals.cpp
)

#set(Boost_DEBUG ON)
#find_package(Boost COMPONENTS ALL)
#set(Boost_DEBUG OFF)

set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.83 REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(math ${Boost_LIBRARIES})
