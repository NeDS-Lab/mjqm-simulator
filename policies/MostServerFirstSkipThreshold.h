//
// Created by mccio on 21/01/25.
//

#ifndef MOSTSERVERFIRSTSKIPTHRESHOLD_H
#define MOSTSERVERFIRSTSKIPTHRESHOLD_H

#include "policy.h"

class MostServerFirstSkipThreshold : public Policy
{
public:
    MostServerFirstSkipThreshold(int w, int servers, int classes, const std::vector<int>& sizes, double arr_s,
                                 double srv_s) :
        servers(servers), w(w), state_buf(classes), state_ser(classes), stopped_jobs(classes), ongoing_jobs(classes),
        sizes(sizes), freeservers(servers), violations_counter(0), drops_below(false), big_priority(false)
    {
        this->threshold = servers - static_cast<int>(sizes[0] * (arr_s / srv_s));
    }
    void arrival(int c, int size, long int id) override;
    void departure(int c, int size, long int id) override;
    const std::vector<int>& get_state_ser() override { return state_ser; }
    const std::vector<int>& get_state_buf() override { return state_buf; }
    const std::vector<std::list<long int>>& get_stopped_jobs() override { return stopped_jobs; }
    const std::vector<std::list<long int>>& get_ongoing_jobs() override { return ongoing_jobs; }
    int get_free_ser() override { return freeservers; }
    int get_window_size() override { return 0; }
    int get_violations_counter() override { return violations_counter; }
    void insert_completion(int size, double completion) override {}
    bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) override { return false; }
    bool prio_big() override { return big_priority; }
    int get_state_ser_small() override;
    void reset_completion(double simtime) override {}
    ~MostServerFirstSkipThreshold() override = default;

private:
    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; // vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; // vector of list of ids
    std::vector<int> sizes;
    int freeservers;
    int violations_counter;
    int threshold;
    bool drops_below;
    bool big_priority;

    void flush_buffer() override;
};


#endif // MOSTSERVERFIRSTSKIPTHRESHOLD_H
