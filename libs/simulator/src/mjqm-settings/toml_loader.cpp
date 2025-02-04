//
// Created by Marco Ciotola on 30/01/25.
//

#include <iostream>
#include <ranges>
#include <unordered_map>

#include <mjqm-policy/policies.h>
#include <mjqm-settings/toml_loader.h>
#include <mjqm-settings/toml_distributions_loaders.h>
#include <mjqm-settings/toml_policies_loaders.h>
#include <mjqm-settings/toml_utils.h>
#include <mjqm-simulator/simulator.h>

unsigned int ExperimentConfig::get_sizes(std::vector<unsigned int>& sizes) const {
    sizes.reserve(classes.size());
    sizes.clear();
    for (const auto& class_config : classes) {
        sizes.push_back(class_config.cores);
    }
    return classes.size();
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
