//
// Created by mccio on 21/01/25.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include <list>
#include <unordered_map>
#include <vector>

class Policy
{
public:
    virtual void arrival(int c, int size, long int id) = 0;
    virtual void departure(int c, int size, long int id) = 0;
    virtual const std::vector<int>& get_state_ser() = 0;
    virtual const std::vector<int>& get_state_buf() = 0;
    virtual const std::vector<std::list<long int>>& get_stopped_jobs() = 0;
    virtual const std::vector<std::list<long int>>& get_ongoing_jobs() = 0;
    virtual int get_free_ser() = 0;
    virtual int get_window_size() = 0;
    virtual int get_violations_counter() = 0;
    virtual void flush_buffer() = 0;
    virtual void insert_completion(int size, double completion) = 0;
    virtual void reset_completion(double simtime) = 0;
    virtual bool fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) = 0;
    virtual bool prio_big() = 0;
    virtual int get_state_ser_small() = 0;
    virtual ~Policy() = 0;
};

inline Policy::~Policy() = default;

#endif // INTERFACE_H
