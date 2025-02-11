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
    std::uniform_real_distribution<double> uniform{0.0, 1.0};

public:
    explicit random_mersenne(std::shared_ptr<std::mt19937_64> generator, const std::string& name) :
        random_source(name), generator(std::move(generator)) {}
    double RandU01() override;
};

static constexpr std::uint64_t next(const std::uint64_t u) {
    std::uint64_t v = u * 3935559000370003845 + 2691343689449507681;

    v ^= v >> 21;
    v ^= v << 37;
    v ^= v >> 4;

    v *= 4768777513237032717;

    v ^= v << 20;
    v ^= v >> 41;
    v ^= v << 5;

    return v;
}

class random_mersenne_factory_shared final : public random_source_factory {
    std::shared_ptr<std::mt19937_64> generator;

public:
    explicit random_mersenne_factory_shared(const std::uint64_t seed = MJQM_RANDOM_MERSENNE_SEED) :
        generator(std::make_shared<std::mt19937_64>(next(seed))) {}
    std::shared_ptr<random_source> create(const std::string& name) override {
        return std::make_shared<random_mersenne>(generator, name);
    }
    ~random_mersenne_factory_shared() override = default;
};

class random_mersenne_factory final : public random_source_factory {
    std::uint64_t seed;

public:
    explicit random_mersenne_factory(const std::uint64_t seed = MJQM_RANDOM_MERSENNE_SEED) : seed(seed) {}
    std::shared_ptr<random_source> create(const std::string& name) override {
        seed = next(seed);
        return std::make_shared<random_mersenne>(std::make_shared<std::mt19937_64>(seed), name);
    }
    ~random_mersenne_factory() override = default;
};

#endif // MJQM_RANDOM_MERSENNE_H
