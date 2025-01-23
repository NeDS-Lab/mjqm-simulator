//
// Created by mccio on 21/01/25.
//

#include "Smash.h"
Smash::Smash(int w, int servers, int classes)
{
    this->w = w;
    this->violations_counter = 0;
    this->servers = freeservers = servers;
    this->state_buf.resize(classes);
    this->state_ser.resize(classes);
    this->stopped_jobs.resize(classes);
    this->ongoing_jobs.resize(classes);
}
void Smash::arrival(int c, int size, long int id)
{
    std::tuple<int, int, long int> e(c, size, id);
    this->buffer.push_back(e);
    state_buf[c]++;
    flush_buffer();
}
void Smash::departure(int c, int size, long int id)
{
    state_ser[c]--;
    freeservers += size;
    flush_buffer();
}
const std::vector<int>& Smash::get_state_ser() { return state_ser; }
const std::vector<int>& Smash::get_state_buf() { return state_buf; }
const std::vector<std::list<long int>>& Smash::get_stopped_jobs() { return stopped_jobs; }
const std::vector<std::list<long int>>& Smash::get_ongoing_jobs() { return ongoing_jobs; }
int Smash::get_free_ser() { return freeservers; }
int Smash::get_window_size() { return 0; }
int Smash::get_violations_counter() { return violations_counter; }
void Smash::insert_completion(int size, double completion) {}
bool Smash::fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) { return false; }
bool Smash::prio_big() { return false; }
int Smash::get_state_ser_small() { return -1; }
void Smash::reset_completion(double simtime) {}
Smash::~Smash() {}
void Smash::flush_buffer()
{
    ongoing_jobs.clear();
    ongoing_jobs.resize(state_buf.size());

    bool modified = true;
    while (modified && buffer.size() > 0 && freeservers > 0)
    {
        auto it = buffer.begin();
        auto max = buffer.end();
        int i = 0;
        modified = false;

        while (it != buffer.end() && (i < w || w == 0))
        { // find maximum
            if (std::get<1>(*it) <= freeservers && (max == buffer.end() || std::get<1>(*it) > std::get<1>(*max)))
            {
                max = it;
            }
            i++;
            it++;
        }

        if (max != buffer.end())
        {
            freeservers -= std::get<1>(*max);
            state_buf[std::get<0>(*max)]--;
            state_ser[std::get<0>(*max)]++;
            ongoing_jobs[std::get<0>(*max)].push_back(std::get<2>(*max));
            if (buffer.begin() != max)
            {
                violations_counter++;
            }
            buffer.erase(max);
            modified = true;
        }
    }
}
