//
// Created by Adityo Anggraito on 21/01/25.
//

#include <iostream>

#include <mjqm-policies/KillSmart.h>

void KillSmart::arrival(int c, int size, long int id) {
    std::tuple<int,int,long int> e(c,size,id);
    this->buffer.push_back(e);
    state_buf[std::get<0>(e)]++;
    waiting_jobs++;
    flush_buffer();
}
void KillSmart::departure(int c, int size, long int id) {
    std::tuple<int,int,long int> e(c,size,id);
    service_jobs--;
    state_ser[std::get<0>(e)]--;
    freeservers+=std::get<1>(e);
    // remove departing jobs
    this->ongoing_jobs[std::get<0>(e)].remove(std::get<2>(e));
    flush_buffer();
}

void KillSmart::put_jobs_normally() {
    auto it = buffer.begin();
    bool modified = true;
    //std::cout << freeservers << std::endl;
    while (freeservers > 0 && it != buffer.end() && modified == true) {
        if (freeservers >= std::get<1>(*it)) {
            freeservers -= std::get<1>(*it);
            state_ser[std::get<0>(*it)]++;
            state_buf[std::get<0>(*it)]--;
            ongoing_jobs[std::get<0>(*it)].push_back(std::get<2>(*it));
            service_jobs++;
            waiting_jobs--;
            it = buffer.erase(it);
            modified = true;
        } else {
            modified = false;
            it++;
        }
        //it++;
        //state_buf[it->first] --;
    }
}

void KillSmart::flush_buffer() {

    //ongoing_jobs[i].push_back(*it);
    //it = stopped_jobs[i].erase(it);
    int servers_occupied = servers - freeservers;
    if (service_jobs >=1 && service_jobs <= kill_threshold && (!buffer.empty()) && std::get<1>(buffer.front()) > freeservers && std::get<1>(buffer.front()) > servers_occupied) {
        // put all ongoing jobs into stopped
        //std::cout << "KILLING TIME" << std::endl;
        violations_counter++;
        for (int i = 0; i < ongoing_jobs.size(); i++) {
            for (auto it = ongoing_jobs[i].begin(); it != ongoing_jobs[i].end(); ) {
                // kill jobs if killing it won't make us overflow
                if ((stopped_size + sizes[i]) <= max_stopped_size) {
                    //std::cout << "KILLED" << std::endl;
                    //std::cout << ongoing_jobs[i].size() << std::endl;
                    stopped_jobs[i].push_back(*it);
                    it = ongoing_jobs[i].erase(it);

                    service_jobs--;
                    state_ser[i]--;
                    freeservers += sizes[i];

                    stopped_size += sizes[i];
                } else {
                    reach_max_stopped_size = true;
                    after_kill = true;
                    break;
                }
            }
        }
        after_kill = true;
    }

    if (after_kill) {
        put_jobs_normally();
    }

    bool empty_state = (service_jobs == 0 && waiting_jobs == 0);
    bool kill_wins = (service_jobs == 0 && (!buffer.empty()) && std::get<1>(buffer.front()) < stopped_size);
    if (reach_max_stopped_size || empty_state) {
        // not letting any jobs untill we have enough servers to accomodate stopped jobs
        if (freeservers < stopped_size) {
            after_kill = false;
            return;
        } else {
        // put all stopped jobs back in action
            //std::cout <<  stopped_size <<std::endl;
            for (int i = 0; i < stopped_jobs.size(); i++) {
                for (auto it = stopped_jobs[i].begin(); it != stopped_jobs[i].end(); ) {
                    ongoing_jobs[i].push_back(*it);
                    it = stopped_jobs[i].erase(it);

                    service_jobs++;
                    state_ser[i]++;
                    freeservers -= sizes[i];

                    stopped_size -= sizes[i];
                }
            }
            reach_max_stopped_size = false;
            if (freeservers > 0) {
                put_jobs_normally();
            }
        }
    } else {
        put_jobs_normally();
    }
    
    after_kill = false;
}
