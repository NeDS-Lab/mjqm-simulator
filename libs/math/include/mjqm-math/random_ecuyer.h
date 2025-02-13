#ifndef MJQM_RANDOM_ECUYER_H
#define MJQM_RANDOM_ECUYER_H

#include <RngStream.h>

#ifndef MJQM_RANDOM_ECUYER_SEED
#define MJQM_RANDOM_ECUYER_SEED {1034567891, 1123456789, 1276543217, 1346798521, 1526374819, 1987654321}
#endif

// The library is not thread-safe for streams generation:
// we need to initialize the package seed when starting a new simulation.
// Once the streams are created, the package seed is not used anymore
void RngStreamRestart() {
    const long unsigned int initSeed[6] = MJQM_RANDOM_ECUYER_SEED;
    RngStream::SetPackageSeed(initSeed);
}

#endif // MJQM_RANDOM_ECUYER_H
