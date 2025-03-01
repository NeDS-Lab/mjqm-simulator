add_library(math
        src/mjqm-math/confidence_intervals.cpp
)
target_link_libraries(math ${Boost_LIBRARIES})
