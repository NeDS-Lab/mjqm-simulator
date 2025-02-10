#include <mjqm-math/random_mersenne.h>

inline double random_mersenne::RandU01() {
    return static_cast<double>((*generator)()) / static_cast<double>(std::mt19937_64::max());
}

inline long random_mersenne::RandInt(const long low, const long high) {
    return (high - low + 1L) * static_cast<long>((*generator)()) + low;
}
// inline void random_mersenne::setSeed(const uint64_t seed) {
//     generator->seed(seed);
// }
