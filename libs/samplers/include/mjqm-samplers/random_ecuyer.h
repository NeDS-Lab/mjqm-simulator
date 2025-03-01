#ifndef MJQM_RANDOM_ECUYER_H
#define MJQM_RANDOM_ECUYER_H

#include <RngStream.h>
#include <mutex>

#ifndef MJQM_RANDOM_ECUYER_SEED_FROM_ENV
#define MJQM_RANDOM_ECUYER_SEED_FROM_ENV {1034567891, 1123456789, 1276543217, 1346798521, 1526374819, 1987654321}
#endif

// The library is not thread-safe for streams generation:
// we need to initialize the package seed when starting a new simulation.
// Once the streams are created, the package seed is not used anymore
std::mutex RNG_STREAMS_GENERATION_LOCK;
const long unsigned MJQM_RANDOM_ECUYER_SEED[6] = MJQM_RANDOM_ECUYER_SEED_FROM_ENV;

#endif // MJQM_RANDOM_ECUYER_H
