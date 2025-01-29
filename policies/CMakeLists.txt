add_library(policies
        BackFilling.cpp
        BackFilling.h
        MostServerFirst.cpp
        MostServerFirst.h
        MostServerFirstSkip.cpp
        MostServerFirstSkip.h
        MostServerFirstSkipThreshold.cpp
        MostServerFirstSkipThreshold.h
        policies.h
        policy.h
        ServerFilling.cpp
        ServerFilling.h
        ServerFillingMem.cpp
        ServerFillingMem.h
        Smash.cpp
        Smash.h
)

#if(EXAMPLE_PROJECT_BUILD_TESTING)
#    add_subdirectory(tests)
#endif()
