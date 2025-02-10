#ifndef MJQM_RANDOM_MERSENNE_H
#define MJQM_RANDOM_MERSENNE_H

#include <memory>
#include <mjqm-math/random.h>
#include <random>
#include <string>

#ifndef MJQM_RANDOM_MERSENNE_SEED
#define MJQM_RANDOM_MERSENNE_SEED 1862248485
#endif

class random_mersenne final : public random_source {
private:
    const std::shared_ptr<std::mt19937_64> generator;

public:
    explicit random_mersenne(std::shared_ptr<std::mt19937_64> generator, const std::string& name) :
        random_source(name), generator(std::move(generator)) {}
    double RandU01() override;
    long RandInt(long low, long high) override;
};

#endif // MJQM_RANDOM_MERSENNE_H
