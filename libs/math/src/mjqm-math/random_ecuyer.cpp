#include <mjqm-math/random_ecuyer.h>

constexpr long unsigned int initSeed[6] = MJQM_RANDOM_ECUYER_SEED;
const bool RngStream_seed_set = RngStream::SetPackageSeed(initSeed);

inline double random_ecuyer::RandU01() { return generator.RandU01(); }

inline long random_ecuyer::RandInt(const long low, const long high) {
    return generator.RandInt(static_cast<int>(low), static_cast<int>(high));
}
// inline void random_ecuyer::setSeed(unsigned long seed[6]) {
//     generator.SetSeed(seed);
// }
