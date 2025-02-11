#ifndef MJQM_RANDOM_H
#define MJQM_RANDOM_H

#include <memory>
#include <string>
#include <utility>

class random_source {
public:
    virtual ~random_source() = default;
    const std::string name;

    explicit random_source(std::string name) : name(std::move(name)) {}

    virtual double RandU01() = 0;
    long RandInt(const long low, const long high) {
        // the + 1L is needed to give the higher value the same probability as any other value
        return low + (high - low + 1L) * static_cast<long>(RandU01());
    }
};

class random_source_factory {
public:
    virtual ~random_source_factory() = default;
    virtual std::shared_ptr<random_source> create(const std::string& name) = 0;
};

#endif // MJQM_RANDOM_H
