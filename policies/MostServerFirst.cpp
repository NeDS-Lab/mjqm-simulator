//
// Created by mccio on 21/01/25.
//

#include "MostServerFirst.h"
MostServerFirst::MostServerFirst(int w, int servers, int classes, const std::vector<int>& sizes)
{
    this->w = w;
    this->violations_counter = 0;
    this->servers = freeservers = servers;
    this->state_buf.resize(classes);
    this->state_ser.resize(classes);
    this->stopped_jobs.resize(classes);
    this->ongoing_jobs.resize(classes);
    this->sizes = sizes;
}
void MostServerFirst::arrival(int c, int size, long int id)
{
    state_buf[c]++;
    stopped_jobs[c].push_back(id);
    flush_buffer();
}
void MostServerFirst::departure(int c, int size, long int id)
{
    state_ser[c]--;
    freeservers += size;
    flush_buffer();
}
const std::vector<int>& MostServerFirst::get_state_ser() { return state_ser; }
const std::vector<int>& MostServerFirst::get_state_buf() { return state_buf; }
const std::vector<std::list<long int>>& MostServerFirst::get_stopped_jobs() { return stopped_jobs; }
const std::vector<std::list<long int>>& MostServerFirst::get_ongoing_jobs() { return ongoing_jobs; }
int MostServerFirst::get_free_ser() { return freeservers; }
int MostServerFirst::get_window_size() { return 0; }
int MostServerFirst::get_violations_counter() { return violations_counter; }
void MostServerFirst::insert_completion(int size, double completion) {}
bool MostServerFirst::fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) { return false; }
bool MostServerFirst::prio_big() { return false; }
int MostServerFirst::get_state_ser_small()
{
    int tot_small_ser = 0;
    for (int i = 0; i < servers - 1; i++)
    {
        tot_small_ser += state_ser[i];
    }
    return tot_small_ser;
}
void MostServerFirst::reset_completion(double simtime) {}
MostServerFirst::~MostServerFirst() {}
void MostServerFirst::flush_buffer()
{

    ongoing_jobs.clear();
    ongoing_jobs.resize(state_buf.size());

    bool modified = true;
    // bool zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
    while (modified && freeservers > 0)
    {
        modified = false;
        for (int i = state_buf.size() - 1; i >= 0; --i)
        {
            auto it = stopped_jobs[i].begin();
            while (state_buf[i] != 0 && sizes[i] <= freeservers)
            {
                state_buf[i]--;
                state_ser[i]++;
                ongoing_jobs[i].push_back(*it);
                it = stopped_jobs[i].erase(it);
                freeservers -= sizes[i];
                modified = true;
            }
        }
        // zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
    }
}
