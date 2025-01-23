//
// Created by mccio on 21/01/25.
//

#ifndef BACKFILLING_H
#define BACKFILLING_H
#include "policy.h"

#include <map>


class BackFilling final : public Policy
{
public:
    BackFilling(int w, int servers, int classes, const std::vector<int>& sizes);
    void arrival(int c, int size, long int id) override;
    void departure(int c, int size, long int id) override;
    bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) override;
    const std::vector<int>& get_state_ser() override;
    const std::vector<int>& get_state_buf() override;
    const std::vector<std::list<long int>>& get_stopped_jobs() override;
    const std::vector<std::list<long int>>& get_ongoing_jobs() override;
    int get_free_ser() override;
    int get_window_size() override;
    int get_violations_counter() override;
    void insert_completion(int size, double completion) override;
    void reset_completion(double simtime) override;
    bool prio_big() override;
    int get_state_ser_small() override;
    ~BackFilling() override;

private:
    std::list<std::tuple<int, int, long int>> buffer;
    std::list<std::tuple<int, int, long int>> mset; // list of jobs in service
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; // vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; // vector of list of ids
    int freeservers;
    int servers;
    int w;
    bool debugMode;
    std::vector<int> sizes;
    std::map<double, int> completion_time;
    int violations_counter;

    double schedule_next();
    void flush_buffer() override;
};


#endif // BACKFILLING_H
