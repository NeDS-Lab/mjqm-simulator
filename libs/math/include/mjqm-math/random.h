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
    virtual long RandInt(const long low, const long high) {
        return low + static_cast<long>((static_cast<double>(high) - static_cast<double>(low) + 1.0) * RandU01());
    }
};

class random_source_factory {
public:
    virtual ~random_source_factory() = default;
    virtual std::shared_ptr<random_source> create(const std::string& name) = 0;
};

#endif // MJQM_RANDOM_H
