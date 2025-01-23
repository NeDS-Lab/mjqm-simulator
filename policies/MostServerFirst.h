//
// Created by mccio on 21/01/25.
//

#ifndef MOSTSERVERFIRST_H
#define MOSTSERVERFIRST_H
#include "policy.h"


class MostServerFirst : public Policy
{
public:
    MostServerFirst(int w, int servers, int classes, const std::vector<int>& sizes);
    void arrival(int c, int size, long int id);
    void departure(int c, int size, long int id);
    const std::vector<int>& get_state_ser();
    const std::vector<int>& get_state_buf();
    const std::vector<std::list<long int>>& get_stopped_jobs();
    const std::vector<std::list<long int>>& get_ongoing_jobs();
    int get_free_ser();
    int get_window_size();
    int get_violations_counter();
    void insert_completion(int size, double completion);
    bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime);
    bool prio_big();
    int get_state_ser_small();
    void reset_completion(double simtime);
    ~MostServerFirst();

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


    void flush_buffer();
};

#endif //MOSTSERVERFIRST_H
