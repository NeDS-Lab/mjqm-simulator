#ifndef MJQM_RANDOM_ECUYER_H
#define MJQM_RANDOM_ECUYER_H

#include <RngStream.h>
#include <mjqm-math/random.h>
#include <string>

#ifndef MJQM_RANDOM_ECUYER_SEED
#define MJQM_RANDOM_ECUYER_SEED {1034567891, 1123456789, 1276543217, 1346798521, 1526374819, 1987654321}
#endif

class random_ecuyer final : public random_source {
private:
    RngStream generator;

public:
    explicit random_ecuyer(const std::string& name) : random_source(name), generator(name.data()) {}
    double RandU01() override;
};

class random_ecuyer_factory final : public random_source_factory {
public:
    random_ecuyer_factory() {
        // the library is not thread-safe for streams generation, so we better initialize the package seed every time
        // once the streams are created, the package seed is not used anymore
        const long unsigned int initSeed[6] = MJQM_RANDOM_ECUYER_SEED;
        RngStream::SetPackageSeed(initSeed);
    }
    std::shared_ptr<random_source> create(const std::string& name) override {
        return std::make_shared<random_ecuyer>(name);
    }
};

#endif // MJQM_RANDOM_ECUYER_H
