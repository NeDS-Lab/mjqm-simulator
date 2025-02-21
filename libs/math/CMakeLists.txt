add_library(math
        src/mjqm-math/confidence_intervals.cpp
)
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
