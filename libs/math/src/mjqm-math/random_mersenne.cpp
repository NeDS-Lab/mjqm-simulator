#include <mjqm-math/random_mersenne.h>

inline double random_mersenne::RandU01() {
    return uniform(*generator);
}
