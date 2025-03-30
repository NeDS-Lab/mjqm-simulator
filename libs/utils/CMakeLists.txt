add_library(math
        src/mjqm-math/confidence_intervals.cpp
        src/mjqm-math/mean_var.cpp
)
target_link_libraries(math ${Boost_LIBRARIES})
