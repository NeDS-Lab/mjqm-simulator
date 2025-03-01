# Include the L'Écuyer RngStreams library
set(RngStreams_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "Build using shared libraries")
include(FetchContent)
FetchContent_Declare(
        RngStreams
        GIT_REPOSITORY https://github.com/umontreal-simul/RngStreams.git
        GIT_TAG        b47f576ee28951bc437b3ed686ff4b81a96982c0 # last commit checked on 2025-02
)
FetchContent_MakeAvailable(RngStreams)
