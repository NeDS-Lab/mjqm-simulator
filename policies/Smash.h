//
// Created by mccio on 21/01/25.
//

#ifndef SMASH_H
#define SMASH_H
#include "policy.h"


class Smash : public Policy
{
public:
    Smash(int w, int servers, int classes);
    void arrival(int c, int size, long int id) override;
    void departure(int c, int size, long int id) override;
    const std::vector<int>& get_state_ser() override;
    const std::vector<int>& get_state_buf() override;
    const std::vector<std::list<long int>>& get_stopped_jobs() override;
    const std::vector<std::list<long int>>& get_ongoing_jobs() override;
    int get_free_ser() override;
    int get_window_size() override;
    int get_violations_counter() override;
    void insert_completion(int size, double completion) override;
    bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) override;
    bool prio_big() override;
    int get_state_ser_small() override;
    void reset_completion(double simtime) override;
    ~Smash() override;

private:
    std::list<std::tuple<int, int, long int>> buffer;
    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; // vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; // vector of list of ids
    int freeservers;
    int violations_counter;

    void flush_buffer() override;
};


#endif // SMASH_H
