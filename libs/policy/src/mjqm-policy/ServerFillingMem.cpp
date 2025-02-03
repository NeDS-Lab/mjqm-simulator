//
// Created by Marco Ciotola on 21/01/25.
//

#include <mjqm-policy/ServerFillingMem.h>
#include <iostream>

void ServerFillingMem::arrival(int c, int size, long int id) {
    std::tuple<int, int, long int> e(c, size, id);
    this->buffer.push_back(e);
    ++state_buf[std::get<0>(e)];
    flush_buffer();
}
void ServerFillingMem::departure(int c, int size, long int id) {
    std::tuple<int, int, long int> e(c, size, id);
    --state_ser[std::get<0>(e)];
    freeservers += std::get<1>(e);
    this->mset_coreNeed -= std::get<1>(e);

    auto it = mset.begin();
    while (it != mset.end()) {
        if (std::get<2>(e) == std::get<2>(*it)) {
            it = this->mset.erase(it);
        } else {
            ++it;
        }
    }
    // remove departing jobs
    this->ongoing_jobs[std::get<0>(e)].remove(std::get<2>(e));
    /*auto dep_job = std::find(ongoing_jobs.begin(), ongoing_jobs.end(), std::get<2>(e));
    this->ongoing_jobs.erase(dep_job);*/
    flush_buffer();
}
void ServerFillingMem::addToMset(const std::tuple<int, int, long int>& e) {
    auto it = mset.begin();
    while (it != mset.end() && std::get<1>(e) <= std::get<1>(*it)) {
        ++it;
    }
    mset.insert(it, e);
}
void printList(const std::list<std::pair<int, int>>& myList) {
    for (const auto& pair : myList) {
        std::cout << "(" << pair.first << ", " << pair.second << ") ";
    }
    std::cout << std::endl;
}
void ServerFillingMem::printMset() {
    std::cout << "Mset (class-ids): " << std::endl;
    int size = 0;
    for (auto& e : mset) {
        std::cout << std::get<0>(e) << "-" << std::get<2>(e) << ", ";
        size += std::get<1>(e);
    }
    std::cout << "size: " << size << std::endl;
    std::cout << "Ongoing jobs (ids): " << std::endl;
    for (auto& ongoing_job : ongoing_jobs) {
        for (auto& e : ongoing_job) {
            std::cout << e << ", ";
        }
    }
    std::cout << std::endl;
    std::cout << "Stopped jobs (ids): " << std::endl;
    for (auto& stopped_job : stopped_jobs) {
        for (auto& e : stopped_job) {
            std::cout << e << ", ";
        }
    }
    std::cout << std::endl;
}
void ServerFillingMem::printBuffer() {
    std::cout << "Buffer (class-ids): " << std::endl;
    for (auto& e : buffer) {
        std::cout << std::get<0>(e) << "-" << std::get<2>(e) << ", ";
    }
    std::cout << std::endl;
}
void ServerFillingMem::flush_buffer() {
    if (freeservers > 0) {
        for (int i = 0; i < state_buf.size(); ++i) {
            state_buf[i] += state_ser[i];
        }

        auto it = buffer.begin();
        while (mset_coreNeed < servers && it != buffer.end()) {
            this->addToMset(*it);
            mset_coreNeed += std::get<1>(*it);
            it = buffer.erase(it);
            // it++;
            // state_buf[it->first] --;
        }

        freeservers = servers;
        state_ser.assign(state_buf.size(), 0);
        stopped_jobs.clear();
        ongoing_jobs.clear();
        stopped_jobs.resize(state_buf.size());
        ongoing_jobs.resize(state_buf.size());

        for (auto& elem : mset) {
            if (std::get<1>(elem) <= freeservers) {
                ++state_ser[std::get<0>(elem)];
                --state_buf[std::get<0>(elem)];
                freeservers -= std::get<1>(elem);
                ongoing_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
            } else {
                stopped_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
            }
        }
    }
}
