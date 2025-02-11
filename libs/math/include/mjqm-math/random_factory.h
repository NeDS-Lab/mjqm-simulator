#ifndef MJQM_MATH_RANDOM_FACTORY_H
#define MJQM_MATH_RANDOM_FACTORY_H

#include <memory>
#include <mjqm-math/random.h>
#include <mjqm-math/random_ecuyer.h>
#include <mjqm-math/random_mersenne.h>
#include <string>

template <typename source_t>
class random_source_factory {
public:
    virtual ~random_source_factory() = default;
    virtual std::shared_ptr<source_t> create(const std::string& name) = 0;
};

//

class random_ecuyer_factory final : public random_source_factory<random_ecuyer> {
public:
    std::shared_ptr<random_ecuyer> create(const std::string& name) override {
        return std::make_shared<random_ecuyer>(name);
    }
    ~random_ecuyer_factory() override = default;
};

//

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

class random_mersenne_factory_shared final : public random_source_factory<random_mersenne> {
    std::shared_ptr<std::mt19937_64> generator;

public:
    explicit random_mersenne_factory_shared(const std::uint64_t seed = MJQM_RANDOM_MERSENNE_SEED) :
        generator(std::make_shared<std::mt19937_64>(next(seed))) {}
    std::shared_ptr<random_mersenne> create(const std::string& name) override {
        return std::make_shared<random_mersenne>(generator, name);
    }
    ~random_mersenne_factory_shared() override = default;
};

class random_mersenne_factory final : public random_source_factory<random_mersenne> {
    std::uint64_t seed;

public:
    explicit random_mersenne_factory(const std::uint64_t seed = MJQM_RANDOM_MERSENNE_SEED) : seed(seed) {}
    std::shared_ptr<random_mersenne> create(const std::string& name) override {
        seed = next(seed);
        return std::make_shared<random_mersenne>(std::make_shared<std::mt19937_64>(seed), name);
    }
    ~random_mersenne_factory() override = default;
};
#endif // MJQM_MATH_RANDOM_FACTORY_H
