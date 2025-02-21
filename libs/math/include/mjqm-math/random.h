#ifndef MJQM_RANDOM_H
#define MJQM_RANDOM_H

#include <memory>
#include <string>
#include <utility>

class random_source {
public:
    typedef double result_type;
    const std::string name;

    explicit random_source(std::string name) : name(std::move(name)) {}

    virtual inline double RandU01() = 0;
    static constexpr inline double min() { return 0.0; }
    static constexpr inline double max() { return 1.0; }
    inline double operator()() { return RandU01(); }
};

class random_source_factory {
public:
    virtual ~random_source_factory() = default;
    virtual std::shared_ptr<random_source> create(const std::string& name) = 0;
};

#endif // MJQM_RANDOM_H
