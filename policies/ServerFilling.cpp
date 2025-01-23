//
// Created by mccio on 21/01/25.
//

#include "ServerFilling.h"

#include <tuple>
ServerFilling::ServerFilling(int w, int servers, int classes)
{
    this->w = w;
    this->servers = freeservers = servers;
    this->state_buf.resize(classes);
    this->state_ser.resize(classes);
    this->mset_coreNeed = 0;
    this->stopped_jobs.resize(classes);
    this->ongoing_jobs.resize(classes);
}
void ServerFilling::arrival(int c, int size, long int id)
{
    std::tuple<int, int, long int> e(c, size, id);
    this->buffer.push_back(e);
    state_buf[std::get<0>(e)]++;
    flush_buffer();
}
void ServerFilling::departure(int c, int size, long int id)
{
    std::tuple<int, int, long int> e(c, size, id);
    state_ser[std::get<0>(e)]--;
    freeservers += std::get<1>(e);
    this->mset_coreNeed -= std::get<1>(e);
    auto it = mset.begin();
    while (it != mset.end())
    {
        if (std::get<2>(e) == std::get<2>(*it))
        {
            it = this->mset.erase(it);
        }
        else
        {
            it++;
        }
    }
    // remove departing jobs
    this->ongoing_jobs[std::get<0>(e)].remove(std::get<2>(e));
    flush_buffer();
}
int ServerFilling::get_free_ser() { return freeservers; }
int ServerFilling::get_window_size() { return mset.size(); }
int ServerFilling::get_violations_counter() { return 0; }
void ServerFilling::insert_completion(int size, double completion) {}
bool ServerFilling::prio_big() { return false; }
int ServerFilling::get_state_ser_small() { return -1; }
void ServerFilling::reset_completion(double simtime) {}
void ServerFilling::addToMset(const std::tuple<int, int, long int>& e)
{
    auto it = mset.begin();
    while (it != mset.end() && std::get<1>(e) <= std::get<1>(*it))
    {
        ++it;
    }
    mset.insert(it, e);
}
void ServerFilling::flush_buffer()
{

    if (freeservers > 0)
    {
        auto it = buffer.begin();

        for (int i = 0; i < state_buf.size(); i++)
        {
            state_buf[i] += state_ser[i];
        }

        while (mset_coreNeed < servers && it != buffer.end())
        {
            this->addToMset(*it);
            mset_coreNeed += std::get<1>(*it);
            it = buffer.erase(it);
        }

        freeservers = servers;
        state_ser.assign(state_buf.size(), 0);
        stopped_jobs.clear();
        ongoing_jobs.clear();
        stopped_jobs.resize(state_buf.size());
        ongoing_jobs.resize(state_buf.size());

        for (auto& elem : mset)
        {
            if (std::get<1>(elem) <= freeservers)
            {
                state_ser[std::get<0>(elem)]++;
                state_buf[std::get<0>(elem)]--;
                freeservers -= std::get<1>(elem);
                ongoing_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
            }
            else
            {
                stopped_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
            }
        }
    }
}
