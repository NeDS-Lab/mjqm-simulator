//
// Created by Marco Ciotola on 21/01/25.
//

#ifndef SERVERFILLING_H
#define SERVERFILLING_H

#include <mjqm-policy/policy.h>

class ServerFilling : public Policy {
public:
    ServerFilling(int w, int servers, int classes) :
        state_buf(classes), state_ser(classes), stopped_jobs(classes), ongoing_jobs(classes), mset_coreNeed(0),
        freeservers(servers), servers(servers), w(w) {}
    void arrival(int c, int size, long int id) override;
    void departure(int c, int size, long int id) override;
    const std::vector<int>& get_state_ser() override { return state_ser; };
    const std::vector<int>& get_state_buf() override { return state_buf; }
    const std::vector<std::list<long int>>& get_stopped_jobs() override { return stopped_jobs; }
    const std::vector<std::list<long int>>& get_ongoing_jobs() override { return ongoing_jobs; }
    int get_free_ser() override { return freeservers; }
    int get_window_size() override { return mset.size(); }
    int get_violations_counter() override { return 0; }
    void insert_completion(int size, double completion) override {}
    bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) override { return false; }
    bool prio_big() override { return false; }
    int get_state_ser_small() override { return -1; }
    void reset_completion(double simtime) override {}
    ~ServerFilling() override = default;
    std::unique_ptr<Policy> clone() const override {
        return std::make_unique<ServerFilling>(w, servers, state_buf.size());
    }
    explicit operator std::string() const override {
        return std::string("ServerFilling(servers=") + std::to_string(servers) +
            ", classes=" + std::to_string(state_buf.size()) + ")";
    }

private:
    std::list<std::tuple<int, int, long int>> buffer;
    std::list<std::tuple<int, int, long int>> mset;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; // vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; // vector of list of ids
    int mset_coreNeed;
    int freeservers;
    int servers;
    int w;

    void addToMset(const std::tuple<int, int, long int>& e);
    void flush_buffer() override;
};

#endif // SERVERFILLING_H
