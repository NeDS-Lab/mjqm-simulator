//
// Created by Marco Ciotola on 21/01/25.
//

#include <iostream>

#include <mjqm-policies/FirstFit.h>

void FirstFit::arrival(int c, int size, long int id) {
    std::tuple<int,int,long int> e(c,size,id);
    this->buffer.push_back(e);
    state_buf[std::get<0>(e)]++;
    flush_buffer();
}
void FirstFit::departure(int c, int size, long int id) {
    std::tuple<int,int,long int> e(c,size,id);
    state_ser[std::get<0>(e)]--;
    freeservers+=std::get<1>(e);
    flush_buffer();
}

void FirstFit::flush_buffer() {

    ongoing_jobs.clear();
    ongoing_jobs.resize(state_buf.size());

    if (freeservers > 0) {
        auto it = buffer.begin();
        //std::cout << freeservers << std::endl;
        while (freeservers > 0 && it != buffer.end()) {
            if (freeservers >= std::get<1>(*it)) {
                freeservers -= std::get<1>(*it);
                state_ser[std::get<0>(*it)]++;
                state_buf[std::get<0>(*it)]--;
                ongoing_jobs[std::get<0>(*it)].push_back(std::get<2>(*it));
                it = buffer.erase(it);
            } else {
                it++;
            }
            //it++;
            //state_buf[it->first] --;
        }
        //std::cout << freeservers << std::endl;
    }
}
