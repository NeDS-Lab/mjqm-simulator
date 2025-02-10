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
    long RandInt(long low, long high) override;
};


#endif // MJQM_RANDOM_ECUYER_H
