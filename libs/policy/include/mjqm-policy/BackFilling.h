//
// Created by Marco Ciotola on 21/01/25.
//

#ifndef BACKFILLING_H
#define BACKFILLING_H

#include <map>
#include <mjqm-policy/policy.h>
#include <mjqm-utils/string.hpp>

#define EXPORT_BUILDING_CLASS_SAVE_CONSTRUCT_DATA_VALUE(v)
#define EXPORT_BUILDING_CLASS_SAVE_CONSTRUCT_DATA_VALUES(v, ...)                                                       \
    ar << t->v;                                                                                                        \
    __VA_OPT__(EXPORT_BUILDING_CLASS_SAVE_CONSTRUCT_DATA_VALUES(__VA_ARGS__))

#define EXPORT_BUILDING_CLASS(ClassName, SuperClass, RequiredArgs, OtherValues)                                        \
    template <class Archive>                                                                                           \
    friend void boost::serialization::save_construct_data(Archive& ar, const ClassName* t,                             \
                                                          const unsigned int file_version) {                           \
        EXPORT_BUILDING_CLASS_SAVE_CONSTRUCT_DATA_VALUES##RequiredArgs                                                 \
    }

class BackFilling final : public Policy {
public:
    BackFilling(const int w, const int servers, const int classes, const std::vector<unsigned int>& sizes) :
        state_buf(classes), state_ser(classes), stopped_jobs(classes), ongoing_jobs(classes), freeservers(servers),
        servers(servers), w(w), sizes(sizes), violations_counter(0) {}
    void arrival(int c, int size, long int id) override;
    void departure(int c, int size, long int id) override;
    bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) override;
    const std::vector<int>& get_state_ser() override { return state_ser; }
    const std::vector<int>& get_state_buf() override { return state_buf; }
    const std::vector<std::list<long int>>& get_stopped_jobs() override { return stopped_jobs; }
    const std::vector<std::list<long int>>& get_ongoing_jobs() override { return ongoing_jobs; }
    int get_free_ser() override { return freeservers; }
    int get_window_size() override { return 0; }
    int get_w() const override { return w; }
    int get_violations_counter() override { return violations_counter; }
    void insert_completion(int size, double completion) override;
    void reset_completion(double simtime) override;
    bool prio_big() override { return false; }
    int get_state_ser_small() override { return -1; }
    ~BackFilling() override = default;
    std::unique_ptr<Policy> clone() const override {
        return std::make_unique<BackFilling>(w, servers, state_buf.size(), sizes);
    }
    explicit operator std::string() const override {
        return std::string("BackFilling(servers=") + std::to_string(servers) +
            ", classes=" + std::to_string(state_buf.size()) + ", sizes=(" + join(sizes.begin(), sizes.end()) + "))";
    }

private:
    std::list<std::tuple<int, int, long int>> buffer;
    std::list<std::tuple<int, int, long int>> mset; // list of jobs in service
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; // vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; // vector of list of ids
    int freeservers;
    int servers;
    const int w;
    const std::vector<unsigned int> sizes;
    std::map<double, int> completion_time;
    int violations_counter;

    double schedule_next() const;
    void flush_buffer() override;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, unsigned int) {
        ar.template register_type<BackFilling>();
        ar& boost::serialization::base_object<Policy>(*this);
        ar & buffer;
        ar & mset;
        ar & state_buf;
        ar & state_ser;
        ar & stopped_jobs;
        ar & ongoing_jobs;
        ar & freeservers;
        ar & servers;
        ar & w;
        ar & sizes;
        ar & completion_time;
        ar & violations_counter;
    }
    EXPORT_BUILDING_CLASS(BackFilling, Policy, (w, servers, state_buf.size(), sizes), ())
    template <class Archive>
    friend void boost::serialization::save_construct_data(Archive& ar, const BackFilling* t,
                                                          const unsigned int file_version);

    template <class Archive>
    friend void boost::serialization::load_construct_data(Archive& ar, BackFilling* t, const unsigned int file_version);
};
namespace boost::serialization {
    template <class Archive>
    inline void save_construct_data(Archive& ar, const BackFilling* t, const unsigned int file_version) {
        // save data required to construct instance
        ar << t->w;
        ar << t->servers;
        ar << t->state_buf.size();
        ar << t->sizes;
    }

    template <class Archive>
    inline void load_construct_data(Archive& ar, BackFilling* t, const unsigned int file_version) {
        // retrieve data from archive required to construct new instance
        int w;
        int servers;
        int classes;
        std::vector<unsigned int> sizes;
        ar >> w;
        ar >> servers;
        ar >> classes;
        ar >> sizes;
        // invoke inplace constructor to initialize instance of my_class
        ::new (t) BackFilling(w, servers, classes, sizes);
    }
} // namespace boost::serialization
BOOST_CLASS_EXPORT_IMPLEMENT(BackFilling);

#endif // BACKFILLING_H
