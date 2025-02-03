//
// Created by Marco Ciotola on 30/01/25.
//

#include <iostream>
#include <ranges>
#include <unordered_map>

#include <mjqm-policy/policies.h>
#include <mjqm-settings/toml_loader.h>
#include <mjqm-simulator/simulator.h>

#ifndef XOR
#define XOR(a, b) (!(a) != !(b))
#endif // XOR

#ifndef error_highlight
#define error_highlight(a) BOLDRED << a << RESET << RED
#endif // error_highlight

#ifndef print_error
#define print_error(a) std::cerr << error_highlight("Error: ") << a << RESET << std::endl
#endif // print_error

unsigned int ExperimentConfig::get_sizes(std::vector<unsigned int>& sizes) const {
    sizes.reserve(classes.size());
    sizes.clear();
    for (const auto& class_config : classes) {
        sizes.push_back(class_config.cores);
    }
    return classes.size();
}

template <typename VAR_TYPE>
bool load_into(const toml::table& data, const std::string_view path, VAR_TYPE& value) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    if (val.has_value()) {
        value = val.value();
        return true;
    }
    print_error("Value missing in TOML file " << path);
    return false;
}

template <typename VAR_TYPE>
bool load_into(const toml::table& data, const std::string_view path, VAR_TYPE& value, const VAR_TYPE& def) {
    auto val = data.at_path(path).value<VAR_TYPE>();
    value = val.value_or(def);
    return true;
}

template <typename VAR_TYPE>
VAR_TYPE either(const std::optional<VAR_TYPE>& first, const std::optional<VAR_TYPE>& second) {
    return first.has_value() ? first.value() : second.value();
}

template <typename VAR_TYPE>
const std::optional<VAR_TYPE> either_optional(const std::optional<VAR_TYPE>& first,
                                              const std::optional<VAR_TYPE>& second) {
    return first.has_value() ? first : second;
}

template <typename VAR_TYPE>
const std::optional<VAR_TYPE> either_optional(const toml::node_view<const toml::node>& first,
                                              const toml::node_view<const toml::node>& second) {
    return either_optional(first.value<VAR_TYPE>(), second.value<VAR_TYPE>());
}

template <typename VAR_TYPE>
std::optional<VAR_TYPE> distribution_parameter(const toml::table& data, const std::string_view& cls,
                                               const distribution_use use, const std::string_view& type,
                                               const std::string_view& param) {
    return either_optional<VAR_TYPE>(
        data.at_path(cls).at_path(distribution_use_to_key.at(use)).at_path(param),
        data.at_path("simulation.default").at_path(distribution_use_to_key.at(use)).at_path(type).at_path(param));
}
template <typename VAR_TYPE>
std::optional<VAR_TYPE> distribution_parameter(const toml::table& data, const std::string_view& cls,
                                               const distribution_use use, const std::string_view& param) {
    return either_optional<VAR_TYPE>(
        data.at_path(cls).at_path(distribution_use_to_key.at(use)).at_path(param),
        data.at_path("simulation.default").at_path(distribution_use_to_key.at(use)).at_path(param));
}

bool load_bounded_pareto(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                         std::shared_ptr<std::mt19937_64> generator,
                         std::unique_ptr<sampler>* distribution // out
) {
    auto opt_alpha = distribution_parameter<double>(data, cls, use, "bounded_pareto", "alpha");
    auto opt_mean = distribution_parameter<double>(data, cls, use, "bounded_pareto", "mean");
    auto opt_rate = distribution_parameter<double>(data, cls, use, "bounded_pareto", "rate");
    auto opt_l = distribution_parameter<double>(data, cls, use, "bounded_pareto", "l");
    auto opt_L = distribution_parameter<double>(data, cls, use, "bounded_pareto", "L");
    opt_l = either_optional(opt_l, opt_L);
    auto opt_h = distribution_parameter<double>(data, cls, use, "bounded_pareto", "h");
    auto opt_H = distribution_parameter<double>(data, cls, use, "bounded_pareto", "H");
    opt_h = either_optional(opt_h, opt_H);
    if (!(opt_alpha.has_value() &&
          XOR(XOR(opt_mean.has_value(), opt_rate.has_value()), opt_l.has_value() && opt_h.has_value()))) {
        print_error("Bounded pareto distribution at path "
                    << error_highlight(cls << "." << use)
                    << " must have alpha defined, and either mean, rate or the l/h pair");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = bounded_pareto::with_mean(generator, opt_mean.value(), alpha);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = bounded_pareto::with_rate(generator, opt_rate.value(), alpha);
        return true;
    }
    *distribution = std::make_unique<bounded_pareto>(generator, alpha, opt_l.value(), opt_h.value());
    return true;
}

bool load_deterministic(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                        std::shared_ptr<std::mt19937_64>,
                        std::unique_ptr<sampler>* distribution // out
) {
    const auto opt_value = distribution_parameter<double>(data, cls, use, "exponential", "value");
    const auto opt_mean = distribution_parameter<double>(data, cls, use, "exponential", "mean");
    if (!XOR(opt_value.has_value(), opt_mean.has_value())) {
        print_error("Deterministic distribution at path " << error_highlight(cls << "." << use)
                                                          << " must have exactly one of value or mean defined");
        return false;
    }
    *distribution = deterministic::with_value(either(opt_value, opt_mean));
    return true;
}

bool load_exponential(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                      std::shared_ptr<std::mt19937_64> generator,
                      std::unique_ptr<sampler>* distribution // out
) {
    const auto opt_mean = distribution_parameter<double>(data, cls, use, "exponential", "mean");
    const auto opt_lambda = distribution_parameter<double>(data, cls, use, "exponential", "lambda");
    const auto opt_rate = distribution_parameter<double>(data, cls, use, "exponential", "rate");
    if (!XOR(opt_mean.has_value(), XOR(opt_lambda.has_value(), opt_rate.has_value()))) {
        print_error("Exponential distribution at path " << error_highlight(cls << "." << use)
                                                        << " must have exactly one of mean or lambda/rate defined");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = exponential::with_mean(generator, opt_mean.value());
        return true;
    }
    *distribution = exponential::with_rate(generator, either(opt_lambda, opt_rate));
    return true;
}

bool load_frechet(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::shared_ptr<std::mt19937_64> generator,
                  std::unique_ptr<sampler>* distribution // out
) {
    auto opt_alpha = distribution_parameter<double>(data, cls, use, "frechet", "alpha");
    auto opt_mean = distribution_parameter<double>(data, cls, use, "frechet", "mean");
    auto opt_rate = distribution_parameter<double>(data, cls, use, "frechet", "rate");
    auto opt_s = distribution_parameter<double>(data, cls, use, "frechet", "s");
    auto m = distribution_parameter<double>(data, cls, use, "frechet", "m").value_or(0.);
    if (!(opt_alpha.has_value() && XOR(XOR(opt_mean.has_value(), opt_s.has_value()), opt_rate.has_value()))) {
        print_error("Frechet distribution at path "
                    << error_highlight(cls << "." << use)
                    << " must have alpha defined, and either mean, rate or s, while m has default value 0");
        return false;
    }
    const double alpha = opt_alpha.value();
    if (opt_mean.has_value()) {
        *distribution = frechet::with_mean(generator, opt_mean.value(), alpha, m);
        return true;
    }
    if (opt_rate.has_value()) {
        *distribution = frechet::with_rate(generator, opt_rate.value(), alpha, m);
        return true;
    }
    *distribution = std::make_unique<frechet>(generator, alpha, opt_s.value(), m, true);
    return true;
}

bool load_uniform(const toml::table& data, const std::string_view& cls, const distribution_use& use,
                  std::shared_ptr<std::mt19937_64> generator,
                  std::unique_ptr<sampler>* distribution // out
) {
    auto opt_mean = distribution_parameter<double>(data, cls, use, "uniform", "mean");
    auto opt_variance = distribution_parameter<double>(data, cls, use, "uniform", "variance");
    auto opt_a = distribution_parameter<double>(data, cls, use, "uniform", "a");
    auto opt_min = distribution_parameter<double>(data, cls, use, "uniform", "min");
    opt_min = either_optional(opt_min, opt_a);
    auto opt_b = distribution_parameter<double>(data, cls, use, "uniform", "b");
    auto opt_max = distribution_parameter<double>(data, cls, use, "uniform", "max");
    opt_max = either_optional(opt_max, opt_b);
    if (!XOR(opt_mean.has_value(), opt_min.has_value() && opt_max.has_value() && !opt_variance.has_value())) {
        print_error("Uniform distribution at path " << error_highlight(cls << "." << use)
                                                    << " must have either the pair of a/min and b/max defined, or mean "
                                                       "defined with optional variance (default 1)");
        return false;
    }
    if (opt_mean.has_value()) {
        *distribution = uniform::with_mean(generator, opt_mean.value(), opt_variance.value_or(1.));
        return true;
    }
    *distribution = std::make_unique<uniform>(generator, opt_min.value(), opt_max.value());
    return true;
}

std::unordered_map<std::string_view, distribution_loader> distribution_loaders = {
    {"exponential", load_exponential},
    {"deterministic", load_deterministic},
    {"bounded pareto", load_bounded_pareto},
    {"frechet", load_frechet},
    {"uniform", load_uniform},
};

bool load_distribution(const toml::table& data, const std::string& cls, const distribution_use& use,
                       std::shared_ptr<std::mt19937_64> generator, // we do want it to be copied
                       std::unique_ptr<sampler>* sampler // out
) {
    auto opt_type = distribution_parameter<std::string>(data, cls, use, "distribution"s);
    if (!opt_type.has_value()) {
        print_error("Distribution type missing at path " << error_highlight(cls << "." << use));
        return false;
    }
    const auto type = opt_type.value();
    if (!distribution_loaders.contains(type)) {
        print_error("Unsupported distribution " << error_highlight(type) << " at path "
                                                << error_highlight(cls << "." << use));
        return false;
    }
    return distribution_loaders.at(type)(data, cls, use, generator, sampler);
}

bool load_class_from_toml(const toml::table& data, const std::string& class_name, ExperimentConfig& conf,
                          std::shared_ptr<std::mt19937_64> generator // we do want it to be copied
) {
    const auto full_key = "class."s + class_name;
    unsigned int cores;
    const bool cores_ok = load_into(data, full_key + ".cores"s, cores);
    std::unique_ptr<sampler> arrival_sampler;
    std::unique_ptr<sampler> service_sampler;
    const bool arrival_ok = load_distribution(data, full_key, ARRIVAL, generator, &arrival_sampler);
    const bool service_ok = load_distribution(data, full_key, SERVICE, generator, &service_sampler);
    if (cores_ok && arrival_ok && service_ok) {
        conf.classes.emplace_back(class_name, cores, std::move(arrival_sampler), std::move(service_sampler));
        return true;
    }
    return false;
}

bool normalise_probs(const toml::impl::wrap_node<toml::table>& classes) {
    std::unordered_map<std::string, double> arrival_probs;
    const size_t n_classes = classes.size();
    for (const auto& [key, value] : classes) {
        const auto arrival_prob = value.at_path("arrival.prob").value<double>();
        if (arrival_prob.has_value()) {
            arrival_probs[key.data()] = arrival_prob.value();
        }
    }
    if (!arrival_probs.empty()) {
        if (arrival_probs.size() == n_classes) {
            double sum = 0.0;
            for (const auto p : std::views::values(arrival_probs)) {
                sum += p;
            }
            for (auto& [key, p] : arrival_probs) {
                p /= sum;
                // fix values in-place so they can be correctly read by distribution builder
                classes.at_path(key).at_path("arrival.prob").value<double>().emplace(p);
            }
        } else {
            print_error("Not all classes have the prob property defined. Define it for none or for all.");
            return false;
        }
    }
    return true;
}

typedef std::unique_ptr<Policy> (*policy_builder)(const toml::table& data, const ExperimentConfig& conf);

std::unique_ptr<Policy> smash_builder(const toml::table& data, const ExperimentConfig& conf) {
    const auto window = data.at_path("simulation.smash.window").value<unsigned int>().value_or(1);
    return std::make_unique<Smash>(window, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> server_filling_builder(const toml::table&, const ExperimentConfig& conf) {
    return std::make_unique<ServerFilling>(-1, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> server_filling_mem_builder(const toml::table&, const ExperimentConfig& conf) {
    return std::make_unique<ServerFillingMem>(-2, conf.cores, conf.classes.size());
}

std::unique_ptr<Policy> back_filling_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<BackFilling>(-3, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> most_server_first_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<MostServerFirst>(0, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> most_server_first_skip_builder(const toml::table&, const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    return std::make_unique<MostServerFirstSkip>(-4, conf.cores, n_classes, sizes);
}

std::unique_ptr<Policy> most_server_first_skip_threshold_builder(const toml::table& data,
                                                                 const ExperimentConfig& conf) {
    std::vector<unsigned int> sizes;
    unsigned int n_classes = conf.get_sizes(sizes);
    int default_threshold = static_cast<int>(
        conf.cores - sizes[0] * conf.classes[0].service_sampler->d_mean() / conf.classes[0].arrival_sampler->d_mean());
    int threshold = data.at_path("simulation.msf.threshold").value<int>().value_or(default_threshold);
    return std::make_unique<MostServerFirstSkipThreshold>(-5, conf.cores, n_classes, sizes, threshold);
}

static std::unordered_map<std::string_view, policy_builder> policy_builders = {
    {"smash", smash_builder},
    {"server filling", server_filling_builder},
    {"server filling memoryful", server_filling_mem_builder},
    {"back filling", back_filling_builder},
    {"most server first", most_server_first_builder},
    {"most server first skip", most_server_first_skip_builder},
    {"most server first skip threshold", most_server_first_skip_threshold_builder},
};

bool from_toml(const std::string_view filename, ExperimentConfig& conf) {
    const toml::table data = toml::parse_file(filename);
    bool ok = true;
    ok = ok && load_into(data, "simulation.identifier", conf.name);
    ok = ok && load_into(data, "simulation.events", conf.events);
    ok = ok && load_into(data, "simulation.repetitions", conf.repetitions);
    ok = ok && load_into(data, "simulation.cores", conf.cores);
    ok = ok && load_into(data, "simulation.policy", conf.policy_name, "smash"s);
    ok = ok && load_into(data, "simulation.generator", conf.generator, "mersenne"s);

    const auto class_c = data["class"];
    ok = ok && class_c.is_table();
    auto generator = std::make_shared<std::mt19937_64>();

    if (ok) {
        const auto classes = *class_c.as_table();
        ok = normalise_probs(classes) && ok;
        for (const auto& [key, value] : classes) {
            ok = load_class_from_toml(data, key.data(), conf, generator) && ok;
            // keep going if one soft fails to show all errors
        }
        // sort classes by cores, or alphabetically by name if cores are equal
        std::ranges::sort(conf.classes, [](const ClassConfig& a, const ClassConfig& b) {
            if (a.cores == b.cores) {
                return a.name < b.name;
            }
            return a.cores < b.cores;
        });
    }

    if (!policy_builders.contains(conf.policy_name)) {
        print_error("Unsupported policy " << error_highlight(conf.policy_name));
        return false;
    }
    conf.policy = policy_builders.at(conf.policy_name)(data, conf);

    return ok;
}

Simulator::Simulator(const ExperimentConfig& conf) : nclasses(conf.classes.size()) {
    // this->l = l; // TODO still needed? YES
    // this->u = u; // TODO still needed? YES
    this->n = conf.cores;
    // this->w = w; // TODO still needed? Y/N (should transform all things that need it)
    // this->sampling_method = sampling_method; // TODO still needed?
    this->rep_free_servers_distro = std::vector<double>(conf.cores + 1);
    this->fel.resize(nclasses * 2);
    this->job_fel.resize(nclasses * 2);
    this->jobs_inservice.resize(nclasses);
    this->jobs_preempted.resize(nclasses);
    this->curr_job_seq.resize(nclasses);
    this->tot_job_seq.resize(nclasses);
    this->curr_job_seq_start.resize(nclasses);
    this->tot_job_seq_dur.resize(nclasses);
    this->job_seq_amount.resize(nclasses);
    this->debugMode = false;
    // this->logfile_name = std::move(logfile_name);
    this->policy = conf.policy->clone();

    occupancy_buf.resize(nclasses);
    occupancy_ser.resize(nclasses);
    completion.resize(nclasses);
    preemption.resize(nclasses);
    throughput.resize(nclasses);
    waitingTime.resize(nclasses);
    waitingTimeVar.resize(nclasses);
    rawWaitingTime.resize(nclasses);
    rawResponseTime.resize(nclasses);
    responseTime.resize(nclasses);
    responseTimeVar.resize(nclasses);
    waste = 0;
    viol = 0;
    util = 0;
    occ = 0;
    std::uint64_t seed = 1862248485;
    generator = std::make_shared<std::mt19937_64>(next(seed));

    for (const auto& cls : conf.classes) {
        sizes.push_back(cls.cores);
        ser_time_samplers.push_back(cls.service_sampler->clone(generator));
        arr_time_samplers.push_back(cls.arrival_sampler->clone(generator));
    }
}
