//
// Created by mccio on 26/01/25.
//

#ifndef TOML_LOADER_H
#define TOML_LOADER_H

#define TOML_ENABLE_UNRELEASED_FEATURES 1

#define RESET "\033[0m"
#define BLACK "\033[30m" // Black
#define RED "\033[31m" // Red
#define GREEN "\033[32m" // Green
#define YELLOW "\033[33m" // Yellow
#define BLUE "\033[34m" // Blue
#define MAGENTA "\033[35m" // Magenta
#define CYAN "\033[36m" // Cyan
#define WHITE "\033[37m" // White
#define BOLDBLACK "\033[1m\033[30m" // Bold Black
#define BOLDRED "\033[1m\033[31m" // Bold Red
#define BOLDGREEN "\033[1m\033[32m" // Bold Green
#define BOLDYELLOW "\033[1m\033[33m" // Bold Yellow
#define BOLDBLUE "\033[1m\033[34m" // Bold Blue
#define BOLDMAGENTA "\033[1m\033[35m" // Bold Magenta
#define BOLDCYAN "\033[1m\033[36m" // Bold Cyan
#define BOLDWHITE "\033[1m\033[37m" // Bold White
#define XOR(a, b) (!(a) != !(b))

#include "toml++/toml.hpp"

using namespace std::string_literals;

class DistributionConfig {
public:
    const std::string name;
    // Constructor to initialize the name
    explicit DistributionConfig(const std::string& name) : name(name) {}
    explicit DistributionConfig(const toml::parse_result& data, const std::string& key);
    virtual bool is_valid() const { return !name.empty(); }
    virtual std::string to_string() const { return name + " distribution"; }
    virtual ~DistributionConfig() = default;
};

class ExponentialDistributionConfig : public DistributionConfig {
public:
    explicit ExponentialDistributionConfig() : DistributionConfig("exponential"s) {}
    explicit ExponentialDistributionConfig(const toml::parse_result& data, const std::string& key) :
        ExponentialDistributionConfig() {
        mean = data.at_path(key + ".mean").value<double>();
        lambda = data.at_path(key + ".lambda").value<double>();
        prob = data.at_path(key + ".prob").value<double>();
        rate = data.at_path(key + ".rate").value<double>();
    }
    std::optional<double> mean = std::nullopt;
    std::optional<double> lambda = std::nullopt;
    std::optional<double> rate = std::nullopt;
    std::optional<double> prob = std::nullopt;
    bool is_valid() const override {
        return DistributionConfig::is_valid() && XOR(XOR(mean.has_value(), lambda.has_value()), rate.has_value());
    }
    std::string to_string() const override {
        return DistributionConfig::to_string() + " with" +
            (!mean.has_value() && !lambda.has_value() && !rate.has_value()
                 ? " unknown mean/lambda/rate"
                 : (mean.has_value() ? " mean=" + std::to_string(mean.value()) : "") +
                     (lambda.has_value() ? " lambda=" + std::to_string(lambda.value()) : "") +
                     (rate.has_value() ? " rate=" + std::to_string(rate.value()) : "")) +
            (prob.has_value() ? " and prob=" + std::to_string(prob.value()) : "");
    }
};

class FrechetDistributionConfig : public DistributionConfig {
public:
    explicit FrechetDistributionConfig() : DistributionConfig("frechet"s) {}
    explicit FrechetDistributionConfig(const toml::parse_result& data, const std::string& key) :
        FrechetDistributionConfig() {
        mean = data.at_path(key + ".mean").value<double>();
        alpha = data.at_path(key + ".alpha").value<double>();
        s = data.at_path(key + ".s").value<double>();
        m = data.at_path(key + ".m").value<double>();
        if (!m.has_value()) {
            m.emplace(0.);
        }
    }
    std::optional<double> mean = std::nullopt;
    std::optional<double> alpha = std::nullopt;
    std::optional<double> s = std::nullopt;
    std::optional<double> m = std::nullopt;
    bool is_valid() const override {
        return DistributionConfig::is_valid() && alpha.has_value() && m.has_value() &&
            XOR(mean.has_value(), s.has_value());
    }
    std::string to_string() const override {
        return DistributionConfig::to_string() + " with " + ("m=" + std::to_string(m.value())) +
            (!(mean.has_value() || s.has_value()) ? " ; unknown mean/s" : "") +
            (mean.has_value() ? " ; mean=" + std::to_string(mean.value()) : "") +
            (s.has_value() ? " ; s=" + std::to_string(s.value()) : "") +
            (alpha.has_value() ? " ; alpha=" + std::to_string(alpha.value()) : " ; unknown alpha");
    }
};

class BoundedParetoDistributionConfig : public DistributionConfig {
public:
    explicit BoundedParetoDistributionConfig() : DistributionConfig("bounded pareto"s) {}
    explicit BoundedParetoDistributionConfig(const toml::parse_result& data, const std::string& key) :
        BoundedParetoDistributionConfig() {
        alpha = data.at_path(key + ".alpha").value<double>();
        mean = data.at_path(key + ".mean").value<double>();
        rate = data.at_path(key + ".rate").value<double>();
        l = data.at_path(key + ".l").value<double>();
        h = data.at_path(key + ".h").value<double>();
    }
    std::optional<double> alpha = std::nullopt;
    std::optional<double> mean = std::nullopt;
    std::optional<double> rate = std::nullopt;
    std::optional<double> l = std::nullopt;
    std::optional<double> h = std::nullopt;
    bool is_valid() const override {
        return DistributionConfig::is_valid() && alpha.has_value() &&
            XOR(XOR(mean.has_value(), rate.has_value()), l.has_value() && h.has_value());
    }
    std::string to_string() const override {
        return DistributionConfig::to_string() + " with " +
            (alpha.has_value() ? "alpha=" + std::to_string(alpha.value()) : "unknown alpha") +
            (!(mean.has_value() || rate.has_value() || (l.has_value() && h.has_value())) ? " ; unknown mean/rate/l+h"
                                                                                         : "") +
            (mean.has_value() ? " ; mean=" + std::to_string(mean.value()) : "") +
            (rate.has_value() ? " ; rate=" + std::to_string(rate.value()) : "") +
            (l.has_value() ? " ; l=" + std::to_string(l.value()) : "") +
            (h.has_value() ? " ; h=" + std::to_string(h.value()) : "");
    }
};

struct ClassConfig {
    std::string name;
    unsigned int cores;
    DistributionConfig* arrival_distribution;
    DistributionConfig* service_distribution;
    ~ClassConfig() = default;
    // {
    //     delete arrival_distribution;
    //     delete service_distribution;
    // };
};

struct ExperimentConfig {
    std::string name;
    unsigned int events;
    unsigned int repetitions;
    unsigned int cores;
    std::string policy;
    std::string default_arrival_distribution;
    std::string default_service_distribution;
    std::vector<ClassConfig> classes;
};

template <typename VAR_TYPE>
bool load(const toml::parse_result& data, std::string_view path, VAR_TYPE& value);

template <typename VAR_TYPE>
bool load(const toml::parse_result& data, std::string_view path, VAR_TYPE& value, const VAR_TYPE& def);

bool load_distribution(const toml::parse_result& data, const std::string& path,
                              DistributionConfig*& distribution, const std::string& def);

bool load_class_from_toml(const toml::parse_result& data, const std::string& key, ExperimentConfig& conf);

bool from_toml(std::string_view filename, ExperimentConfig& conf);

#endif // TOML_LOADER_H
