//
// Created by Marco Ciotola on 21/01/25.
//

#include <iostream>

#include <mjqm-policy/BackFilling.h>

void BackFilling::arrival(int c, int size, long int id) {
    std::tuple<int, int, long int> e(c, size, id);
    this->buffer.push_back(e);
    state_buf[std::get<0>(e)]++;
    flush_buffer();
}
void BackFilling::departure(int c, int size, long int id) {
    std::tuple<int, int, long int> e(c, size, id);
    state_ser[std::get<0>(e)]--;
    freeservers += std::get<1>(e);

    auto it = mset.begin();
    while (it != mset.end()) {
        if (std::get<2>(e) == std::get<2>(*it)) {
            it = this->mset.erase(it);
        }
        ++it;
    }

    // std::cout << completion_time.size() << std::endl;
    if (!completion_time.empty()) {
        completion_time.erase(completion_time.begin());
    } else {
        std::cout << "empty completion_time?" << std::endl;
        violations_counter++;
    }
    // remove departing jobs
    this->ongoing_jobs[std::get<0>(e)].remove(std::get<2>(e));
    /*auto dep_job = std::find(ongoing_jobs.begin(), ongoing_jobs.end(), std::get<2>(e));
    this->ongoing_jobs.erase(dep_job);*/
    flush_buffer();
}
bool BackFilling::fit_jobs(std::unordered_map<long int, double> holdTime, double simTime) {
    bool added = false;
    ongoing_jobs.clear();
    ongoing_jobs.resize(state_buf.size());

    double next_job_start = schedule_next();
    if (next_job_start > 0) {
        for (auto it = buffer.begin(); it != buffer.end();) {
            if (freeservers == 0) {
                break;
            } else if (freeservers > 0 && std::get<1>(*it) <= freeservers &&
                       (holdTime[std::get<2>(*it)] + simTime) <= next_job_start) {
                // insert jobs
                mset.push_back(*it);
                freeservers -= std::get<1>(*it);
                state_ser[std::get<0>(*it)]++;
                state_buf[std::get<0>(*it)]--;
                ongoing_jobs[std::get<0>(*it)].push_back(std::get<2>(*it));
                // delete from buffer
                it = buffer.erase(it);
                added = true;
                // std::cout << "added" << std::endl;
            } else {
                ++it;
            }
        }
    } else {
        // std::cout << "-1" << std::endl;
    }
    return added;
}
void BackFilling::insert_completion(int size, double completion) { completion_time[completion] = size; }
void BackFilling::reset_completion(double simtime) {
    std::map<double, int> new_completion_time;
    for (const auto& ctime : completion_time) {
        new_completion_time[ctime.first - simtime] = ctime.second; // Modify the value associated with each key
    }
    completion_time = new_completion_time;
}
double BackFilling::schedule_next() const {
    auto next_job = buffer.front();
    int next_job_size = std::get<1>(next_job);
    int temp_freeservers = freeservers;
    // std::cout << temp_freeservers << std::endl;
    // std::cout << mset.size() << std::endl;
    for (const auto& ctime : completion_time) {
        temp_freeservers += ctime.second;
        // std::cout << ctime.first << " " << ctime.second << " " << temp_freeservers << " " << next_job_size <<
        // std::endl;
        if (temp_freeservers >= next_job_size) {
            return ctime.first;
        }
    }
    return -1;
}
void BackFilling::flush_buffer() {

    if (freeservers > 0) {
        ongoing_jobs.clear();
        ongoing_jobs.resize(state_buf.size());
        bool modified = true;

        auto it = buffer.begin();
        // std::cout << freeservers << std::endl;
        while (modified && freeservers > 0 && it != buffer.end()) {
            modified = false;
            if (freeservers >= std::get<1>(*it)) {
                mset.push_back(*it);
                freeservers -= std::get<1>(*it);
                state_ser[std::get<0>(*it)]++;
                state_buf[std::get<0>(*it)]--;
                ongoing_jobs[std::get<0>(*it)].push_back(std::get<2>(*it));
                modified = true;
                it = buffer.erase(it);
            }
            // it++;
            // state_buf[it->first] --;
        }
    }
}
