//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <list>
#include <set>
#include <utility>
#include <vector>
#include <random>
#include <chrono>
#include <limits>
#include <float.h>
#include <cmath>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <thread>
#include <ctime>
#include <unordered_map>
#include <pthread.h>
#include <map>
using namespace boost;

struct Confidence_inter{
    double min;
    double max;
    double mean;
};

struct Experiment{
    std::vector<double> l;
    std::vector<double> u;
    std::vector<int> s;
    int w;
    int n;
    int sm;
    std::string logf;
};

class Policy{
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
    virtual bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) = 0;
    virtual bool prio_big() = 0;
    virtual int get_state_ser_small() = 0;
    virtual ~Policy() = 0;
};

Policy::~Policy(){};

class BackFilling: public Policy {
public:
    
    BackFilling(int w, int servers, int classes, const std::vector<int> &sizes) {
        this->w = w;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
        this->debugMode = false;
        this->sizes = sizes;
        this->violations_counter = 0;
    }
    
    void arrival(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        this->buffer.push_back(e);
        state_buf[std::get<0>(e)]++;
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        state_ser[std::get<0>(e)]--;
        freeservers+=std::get<1>(e);

        auto it = mset.begin();
        while (it != mset.end()) {
            if (std::get<2>(e) == std::get<2>(*it)) {
                it = this->mset.erase(it);
            }
            it++;
        }

        //std::cout << completion_time.size() << std::endl;
        if (completion_time.size()>0) {
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

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) {
        bool added = false;
        ongoing_jobs.clear();
        ongoing_jobs.resize(state_buf.size());

        double next_job_start = schedule_next();
        if (next_job_start > 0) {
            for (auto it = buffer.begin(); it != buffer.end(); ) {
                if (freeservers == 0){
                    break;
                } else if (freeservers > 0 && std::get<1>(*it) <= freeservers && (holdTime[std::get<2>(*it)] + simTime) <= next_job_start) {
                    // insert jobs
                    mset.push_back(*it);
                    freeservers -= std::get<1>(*it);
                    state_ser[std::get<0>(*it)]++;
                    state_buf[std::get<0>(*it)]--;
                    ongoing_jobs[std::get<0>(*it)].push_back(std::get<2>(*it));
                    // delete from buffer
                    it = buffer.erase(it);
                    added = true;
                    //std::cout << "added" << std::endl;
                } else {
                    it++;
                }
            }
        } else {
            //std::cout << "-1" << std::endl;
        }
        return added;
    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }

    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return 0;
    }

    int get_violations_counter() {
        return violations_counter;
    }

    void insert_completion(int size, double completion) {
        completion_time[completion] = size;
    }

    void reset_completion(double simtime) {
        std::map<double, int> new_completion_time;
        for (const auto& ctime : completion_time) {
            new_completion_time[ctime.first-simtime] = ctime.second;  // Modify the value associated with each key
        }
        completion_time = new_completion_time;
    }

    bool prio_big(){ return false; }

    int get_state_ser_small() {
        return -1;
    }

    ~BackFilling(){

    }
    
private:
    std::list<std::tuple<int,int,long int>> buffer;
    std::list<std::tuple<int,int,long int>> mset; //list of jobs in service
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    int freeservers;
    int servers;
    int w;
    bool debugMode;
    std::vector<int> sizes;
    std::map<double, int> completion_time;
    int violations_counter;
    

    double schedule_next() {
        auto next_job = buffer.front();
        int next_job_size = std::get<1>(next_job);
        int temp_freeservers = freeservers;
        //std::cout << temp_freeservers << std::endl;
        //std::cout << mset.size() << std::endl;
        for (const auto& ctime : completion_time) {
            temp_freeservers += ctime.second;
            //std::cout << ctime.first << " " << ctime.second << " " << temp_freeservers << " " << next_job_size << std::endl;
            if (temp_freeservers >= next_job_size) {
                return ctime.first;
            }
        }
        return -1;
    }
    
    void flush_buffer() {

        if (freeservers > 0) {
            ongoing_jobs.clear();
            ongoing_jobs.resize(state_buf.size());
            bool modified = true;

            auto it = buffer.begin();
            //std::cout << freeservers << std::endl;
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
                //it++;
                //state_buf[it->first] --;
            }
        }
    }
};


class ServerFillingMem: public Policy {
public:
    
    ServerFillingMem(int w, int servers, int classes) {
        this->w = w;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
        this->mset_coreNeed = 0;
        this->debugMode = false;
    }
    
    void arrival(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        this->buffer.push_back(e);
        state_buf[std::get<0>(e)]++;
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        state_ser[std::get<0>(e)]--;
        freeservers+=std::get<1>(e);
        this->mset_coreNeed -= std::get<1>(e);

        auto it = mset.begin();
        while (it != mset.end()) {
            if (std::get<2>(e) == std::get<2>(*it)) {
                it = this->mset.erase(it);
            } else {
                it++;
            }
        }
        // remove departing jobs
        this->ongoing_jobs[std::get<0>(e)].remove(std::get<2>(e));
        /*auto dep_job = std::find(ongoing_jobs.begin(), ongoing_jobs.end(), std::get<2>(e));
        this->ongoing_jobs.erase(dep_job);*/
        flush_buffer();
    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }

    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return mset.size();
    }

    int get_violations_counter() {
        return 0;
    }

    void insert_completion(int size, double completion) {}

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) { return false; }

    bool prio_big(){ return false; }
    
    int get_state_ser_small() {
        return -1;
    }

    void reset_completion(double simtime) {}

    ~ServerFillingMem(){

    }
    
private:
    std::list<std::tuple<int,int,long int>> buffer;
    std::list<std::tuple<int,int,long int>> mset;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    int mset_coreNeed;
    int freeservers;
    int servers;
    int w;
    bool debugMode;
    

    void addToMset(const std::tuple<int,int,long int>& e) {
        auto it = mset.begin();
        while( it != mset.end() && std::get<1>(e) <= std::get<1>(*it)){
            it ++;
        }
        mset.insert(it, e);
    }
    
    void printList(const std::list<std::pair<int,int>>& myList) {
        for (const auto& pair : myList) {
            std::cout << "(" << pair.first << ", " << pair.second << ") ";
        }
        std::cout << std::endl;
    }

    void printMset() {
        std::cout << "Mset (class-ids): " << std::endl;
        int size = 0;
        for (auto& e : mset) {
            std::cout << std::get<0>(e) << "-" << std::get<2>(e) << ", ";
            size += std::get<1>(e);
        }
        std::cout << "size: " << size << std::endl;
        std::cout << "Ongoing jobs (ids): " << std::endl;
        for (int i = 0; i < ongoing_jobs.size(); i++) {
            for (auto& e : ongoing_jobs[i]) {
                std::cout << e << ", ";
            }
        }
        std::cout << std::endl;
        std::cout << "Stopped jobs (ids): " << std::endl;
        for (int i = 0; i < stopped_jobs.size(); i++) {
            for (auto& e : stopped_jobs[i]) {
                std::cout << e << ", ";
            }
        }
        std::cout << std::endl;
    }

    void printBuffer() {
        std::cout << "Buffer (class-ids): " << std::endl;
        for (auto& e : buffer) {
            std::cout << std::get<0>(e) << "-" << std::get<2>(e) << ", ";
        }
        std::cout << std::endl;
    }
    
    void flush_buffer() {

        if (freeservers > 0) {
            auto it = buffer.begin();

            for (int i = 0; i < state_buf.size(); i++) {
                state_buf[i] += state_ser[i];
            }
            
            while(mset_coreNeed < servers && it != buffer.end()) {
                this->addToMset(*it);
                mset_coreNeed += std::get<1>(*it);
                it = buffer.erase(it);
                //it++;
                //state_buf[it->first] --;
            }

            freeservers = servers;
            state_ser.assign(state_buf.size(), 0);
            stopped_jobs.clear();
            ongoing_jobs.clear();
            stopped_jobs.resize(state_buf.size());
            ongoing_jobs.resize(state_buf.size());

            for (auto& elem: mset) {
                if (std::get<1>(elem) <=  freeservers) {
                    state_ser[std::get<0>(elem)]++;
                    state_buf[std::get<0>(elem)]--;
                    freeservers -= std::get<1>(elem);
                    ongoing_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
                } else {
                    stopped_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
                }
            }
        }
    }
};

class ServerFilling: public Policy {
public:
    
    ServerFilling(int w, int servers, int classes) {
        this->w = w;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->mset_coreNeed = 0;
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
    }
    
    void arrival(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        this->buffer.push_back(e);
        state_buf[std::get<0>(e)]++;
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        state_ser[std::get<0>(e)]--;
        freeservers+=std::get<1>(e);
        this->mset_coreNeed -= std::get<1>(e);
        auto it = mset.begin();
        while (it != mset.end()) {
            if (std::get<2>(e) == std::get<2>(*it)) {
                it = this->mset.erase(it);
            } else {
                it++;
            }
        }
        // remove departing jobs
        this->ongoing_jobs[std::get<0>(e)].remove(std::get<2>(e));
        flush_buffer();
    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }

    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return mset.size();
    }

    int get_violations_counter() {
        return 0;
    }

    void insert_completion(int size, double completion) {}

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) { return false; }

    bool prio_big(){ return false; }

    int get_state_ser_small() {
        return -1;
    }

    void reset_completion(double simtime) {}

    ~ServerFilling(){

    }
    
private:
    std::list<std::tuple<int,int,long int>> buffer;
    std::list<std::tuple<int,int,long int>> mset;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    int mset_coreNeed;
    int freeservers;
    int servers;
    int w;
    

    void addToMset(const std::tuple<int,int,long int>& e) {
        auto it = mset.begin();
        while( it != mset.end() && std::get<1>(e) <= std::get<1>(*it)){
            it ++;
        }
        mset.insert(it, e);
    }
    
    void printList(const std::list<std::pair<int, int>>& myList) {
        for (const auto& pair : myList) {
            std::cout << "(" << pair.first << ", " << pair.second << ") ";
        }
        std::cout << std::endl;
    }
    
    void flush_buffer() {

        if (freeservers > 0) {
            auto it = buffer.begin();

            for (int i = 0; i < state_buf.size(); i++) {
                state_buf[i] += state_ser[i];
            }
            
            while(mset_coreNeed < servers && it != buffer.end()) {
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

            for (auto& elem: mset) {
                if (std::get<1>(elem) <=  freeservers) {
                    state_ser[std::get<0>(elem)]++;
                    state_buf[std::get<0>(elem)]--;
                    freeservers -= std::get<1>(elem);
                    ongoing_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
                } else {
                    stopped_jobs[std::get<0>(elem)].push_back(std::get<2>(elem));
                }
            }

        }
    }
};

class MostServerFirst: public Policy {
public:
    
    MostServerFirst(int w, int servers, int classes, const std::vector<int> &sizes) {
        this->w = w;
        this->violations_counter = 0;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
        this->sizes = sizes;
    }
    
    void arrival(int c, int size, long int id) {
        state_buf[c]++;
        stopped_jobs[c].push_back(id);
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        state_ser[c]--;
        freeservers+=size;
        flush_buffer();
    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }
    
    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return 0;
    }

    int get_violations_counter() {
        return violations_counter;
    }

    void insert_completion(int size, double completion) {}

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) { return false; }

    bool prio_big(){ return false; }

    int get_state_ser_small() {
        int tot_small_ser = 0;
        for (int i = 0; i < servers-1; i++) {
            tot_small_ser += state_ser[i];
        }
        return tot_small_ser;
    }

    void reset_completion(double simtime) {}

    ~MostServerFirst(){

    }

    
private:

    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    std::vector<int> sizes;
    int freeservers;
    int violations_counter;
    
    
    void flush_buffer() {

        ongoing_jobs.clear();
        ongoing_jobs.resize(state_buf.size());

        bool modified = true;
        //bool zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
        while (modified && freeservers > 0) {
            modified = false;
            for (int i = state_buf.size() - 1; i >= 0; --i) {
                auto it = stopped_jobs[i].begin();
                while (state_buf[i] != 0 && sizes[i] <= freeservers) {
                    state_buf[i]--;
                    state_ser[i]++;
                    ongoing_jobs[i].push_back(*it);
                    it = stopped_jobs[i].erase(it);
                    freeservers -= sizes[i];
                    modified = true;
                }
            }
            //zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
        }

    }
};

class MostServerFirstSkip: public Policy {
public:
    
    MostServerFirstSkip(int w, int servers, int classes, const std::vector<int> &sizes) {
        this->w = w;
        this->violations_counter = 0;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
        this->sizes = sizes;
        this->threshold = sizes[0];
        this->big_priority = false;
        this->drops_below = false;
    }
    
    void arrival(int c, int size, long int id) {
        state_buf[c]++;
        stopped_jobs[c].push_back(id);
        if (drops_below && big_priority == false && c == state_buf.size()-1 && state_ser[c] == 0 && get_state_ser_small() > 0) {
            //std::cout << "give to big" << std::endl;
            //std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
            big_priority = true;
        }
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        //std::cout << servers << std::endl;
        state_ser[c]--;
        //std::cout << "keluar " << c << std::endl;
        freeservers+=size;
        if (big_priority && c < state_buf.size()-1 && freeservers >= sizes[sizes.size()-1]) {
            //std::cout << "return to big" << std::endl;
            big_priority = false;
            drops_below = false;
        }
        flush_buffer();
        //std::cout << freeservers << " " << state_buf[state_buf.size()-1] << std::endl;
        //std::cout << drops_below << big_priority << std::endl;
        if (big_priority == false && drops_below == false && c < state_buf.size()-1 && freeservers >= threshold && state_ser[state_ser.size()-1] == 0) {
            //std::cout << "drops below" << std::endl;
            //std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
            drops_below = true;
            if (state_buf[state_buf.size()-1] > 0) {
                //std::cout << "give to big" << std::endl;
                //std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
                big_priority = true;
                //std::cout << state_ser[0] << std::endl;
                if (freeservers >= sizes[sizes.size()-1]) {
                    //std::cout << state_ser[1] << std::endl;
                    std::cout << "masuk lwt bawah" << std::endl;
                    big_priority = false;
                    drops_below = false;
                    flush_buffer();
                    //std::cout << state_ser[1] << std::endl;
                }
                
            }
        } 
        

    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }
    
    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return 0;
    }

    int get_violations_counter() {
        return violations_counter;
    }

    void insert_completion(int size, double completion) {}

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) { return false; }

    bool prio_big(){ return big_priority; }

    int get_state_ser_small() {
        int tot_small_ser = 0;
        for (int i = 0; i < state_ser.size()-1; ++i) {
            tot_small_ser += state_ser[i];
        }
        return tot_small_ser;
    }

    void reset_completion(double simtime) {}

    ~MostServerFirstSkip(){

    }

    
private:

    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    std::vector<int> sizes;
    int freeservers;
    int violations_counter;
    int threshold;
    bool drops_below;
    bool big_priority;
    
    void flush_buffer() {

        ongoing_jobs.clear();
        ongoing_jobs.resize(state_buf.size());

        bool modified = true;
        //bool zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
        while (big_priority == false && modified && freeservers > 0) {
            modified = false;
            for (int i = state_buf.size() - 1; i >= 0; --i) {
                auto it = stopped_jobs[i].begin();
                while (state_buf[i] != 0 && sizes[i] <= freeservers) {
                    state_buf[i]--;
                    state_ser[i]++;
                    ongoing_jobs[i].push_back(*it);
                    it = stopped_jobs[i].erase(it);
                    freeservers -= sizes[i];
                    modified = true;
                }
            }
            //zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
        }

    }
};

class MostServerFirstSkipThreshold: public Policy {
public:
    
    MostServerFirstSkipThreshold(int w, int servers, int classes, const std::vector<int> &sizes, double arr_s, double srv_s) {
        this->w = w;
        this->violations_counter = 0;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
        this->sizes = sizes;
        this->threshold = servers - static_cast<int>(sizes[0]*(arr_s/srv_s));
        this->big_priority = false;
        this->drops_below = false;
    }
    
    void arrival(int c, int size, long int id) {
        state_buf[c]++;
        stopped_jobs[c].push_back(id);
        if (drops_below && big_priority == false && c == state_buf.size()-1 && state_ser[c] == 0 && get_state_ser_small() > 0) {
            //std::cout << "give to big" << std::endl;
            //std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
            big_priority = true;
        }
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        //std::cout << servers << std::endl;
        state_ser[c]--;
        //std::cout << "keluar " << c << std::endl;
        freeservers+=size;
        if (big_priority && c < state_buf.size()-1 && freeservers >= sizes[sizes.size()-1]) {
            //std::cout << "return to big" << std::endl;
            big_priority = false;
            drops_below = false;
        }
        flush_buffer();
        //std::cout << freeservers << " " << state_buf[state_buf.size()-1] << std::endl;
        //std::cout << drops_below << big_priority << std::endl;
        if (big_priority == false && drops_below == false && c < state_buf.size()-1 && freeservers >= threshold && state_ser[state_ser.size()-1] == 0) {
            //std::cout << "drops below" << std::endl;
            //std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
            drops_below = true;
            if (state_buf[state_buf.size()-1] > 0) {
                //std::cout << "give to big" << std::endl;
                //std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
                big_priority = true;
                //std::cout << state_ser[0] << std::endl;
                if (freeservers >= sizes[sizes.size()-1]) {
                    //std::cout << state_ser[1] << std::endl;
                    std::cout << "masuk lwt bawah" << std::endl;
                    big_priority = false;
                    drops_below = false;
                    flush_buffer();
                    //std::cout << state_ser[1] << std::endl;
                }
                
            }
        } 
        

    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }
    
    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return 0;
    }

    int get_violations_counter() {
        return violations_counter;
    }

    void insert_completion(int size, double completion) {}

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) { return false; }

    bool prio_big(){ return big_priority; }

    int get_state_ser_small() {
        int tot_small_ser = 0;
        for (int i = 0; i < state_ser.size()-1; ++i) {
            tot_small_ser += state_ser[i];
        }
        return tot_small_ser;
    }

    void reset_completion(double simtime) {}

    ~MostServerFirstSkipThreshold(){

    }

    
private:

    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    std::vector<int> sizes;
    int freeservers;
    int violations_counter;
    int threshold;
    bool drops_below;
    bool big_priority;
    
    void flush_buffer() {

        ongoing_jobs.clear();
        ongoing_jobs.resize(state_buf.size());

        bool modified = true;
        //bool zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
        while (big_priority == false && modified && freeservers > 0) {
            modified = false;
            for (int i = state_buf.size() - 1; i >= 0; --i) {
                auto it = stopped_jobs[i].begin();
                while (state_buf[i] != 0 && sizes[i] <= freeservers) {
                    state_buf[i]--;
                    state_ser[i]++;
                    ongoing_jobs[i].push_back(*it);
                    it = stopped_jobs[i].erase(it);
                    freeservers -= sizes[i];
                    modified = true;
                }
            }
            //zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
        }

    }
};


class Smash: public Policy {
public:
    
    Smash(int w, int servers, int classes) {
        this->w = w;
        this->violations_counter = 0;
        this->servers = freeservers = servers;
        this->state_buf.resize(classes);
        this->state_ser.resize(classes);
        this->stopped_jobs.resize(classes);
        this->ongoing_jobs.resize(classes);
    }
    
    void arrival(int c, int size, long int id) {
        std::tuple<int,int,long int> e(c,size,id);
        this->buffer.push_back(e);
        state_buf[c]++;
        flush_buffer();
    }
    
    void departure(int c, int size, long int id) {
        state_ser[c]--;
        freeservers+=size;
        flush_buffer();
    }
    
    const std::vector<int>& get_state_ser() {
        return state_ser;
    };
    
    const std::vector<int>& get_state_buf() {
        return state_buf;
    }

    const std::vector<std::list<long int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<long int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_window_size() {
        return 0;
    }

    int get_violations_counter() {
        return violations_counter;
    }

    void insert_completion(int size, double completion) {}

    bool fit_jobs(std::unordered_map<long int,double> holdTime, double simTime) { return false; }

    bool prio_big(){ return false; }

    int get_state_ser_small() {
        return -1;
    }

    void reset_completion(double simtime) {}

    ~Smash(){

    }

    
private:
    std::list<std::tuple<int,int,long int>> buffer;
    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<long int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<long int>> ongoing_jobs; //vector of list of ids
    int freeservers;
    int violations_counter;
    
    
    void flush_buffer() {
        ongoing_jobs.clear();
        ongoing_jobs.resize(state_buf.size());

        bool modified = true;
        while (modified && buffer.size()>0 && freeservers>0) {
            auto it = buffer.begin();
            auto max = buffer.end();
            int i = 0;
            modified = false;
            
            while (it!=buffer.end() && (i<w || w==0)) {//find maximum
                if (std::get<1>(*it) <= freeservers && (max == buffer.end()||std::get<1>(*it)>std::get<1>(*max))) {
                    max = it;
                }
                i++;
                it++;
            }
            
            if (max!=buffer.end()) {
                freeservers -= std::get<1>(*max);
                state_buf[std::get<0>(*max)]--;
                state_ser[std::get<0>(*max)]++;
                ongoing_jobs[std::get<0>(*max)].push_back(std::get<2>(*max));
                if (buffer.begin() != max){
                    violations_counter++;
                } 
                buffer.erase(max);
                modified = true;
            }
        }
    }
};

class Simulator {
public:
    Simulator(const std::vector<double>& l,
              const std::vector<double>& u,
              const std::vector<int>& sizes,
              int w,
              int servers,
              int sampling_method,
              std::string logfile_name): ru(0.0, 1.0) {
        this->l = l;
        this->u = u;
        this->n = servers;
        this->sizes = sizes;
        this->w = w;
        this->sampling_method = sampling_method;
        this->rep_free_servers_distro = std::vector<double>(servers+1);
        this->nclasses = static_cast<int>(sizes.size());
        this->fel.resize(sizes.size()*2);
        this->job_fel.resize(sizes.size()*2);
        this->jobs_inservice.resize(sizes.size());
        this->jobs_preempted.resize(sizes.size());
        this->curr_job_seq.resize(sizes.size());
        this->tot_job_seq.resize(sizes.size());
        this->curr_job_seq_start.resize(sizes.size());
        this->tot_job_seq_dur.resize(sizes.size());
        this->job_seq_amount.resize(sizes.size());
        this->debugMode = false;
        this->logfile_name = logfile_name;

        if (w == 0) {
            this->policy = new  MostServerFirst(w, servers, static_cast<int>(sizes.size()), sizes);
        } else if (w == -1){
            this->policy = new  ServerFilling(w, servers, static_cast<int>(sizes.size()));
        } else if (w == -2){
            this->policy = new  ServerFillingMem(w, servers, static_cast<int>(sizes.size()));
        } else if (w == -3){
            this->policy = new  BackFilling(w, servers, static_cast<int>(sizes.size()), sizes);
        } else if (w == -4){
            this->policy = new  MostServerFirstSkip(w, servers, static_cast<int>(sizes.size()), sizes);
        } else if (w == -5){
            this->policy = new  MostServerFirstSkipThreshold(w, servers, static_cast<int>(sizes.size()), sizes, l[0], 1/u[0]);
        } else {
            this->policy = new  Smash(w, servers, static_cast<int>(sizes.size()));
        }

        occupancy_buf.resize(sizes.size());
        occupancy_ser.resize(sizes.size());
        completion.resize(sizes.size());
        preemption.resize(sizes.size());
        throughput.resize(sizes.size());
        waitingTime.resize(sizes.size());
        waitingTimeVar.resize(sizes.size());
        rawWaitingTime.resize(sizes.size());
        rawResponseTime.resize(sizes.size());
        responseTime.resize(sizes.size());
        responseTimeVar.resize(sizes.size());
        waste = 0;
        viol = 0;
        util = 0;
        occ = 0;
        std::uint64_t seed = 1862248485;
        generator = new std::mt19937_64(static_cast<std::mt19937_64::result_type>(next(seed)));
    
    }
    
    ~Simulator() {
        delete generator;
        delete policy;
    }
    
    void reset_simulation() {
        
        simtime = 0.0;
        
        resample();
    }

    std::uint64_t next(std::uint64_t u) {
        std::uint64_t v = u * 3935559000370003845 + 2691343689449507681;

        v ^= v >> 21;
        v ^= v << 37;
        v ^= v >>  4;

        v *= 4768777513237032717;

        v ^= v << 20;
        v ^= v >> 41;
        v ^= v <<  5;

        return v;
    }

    void reset_statistics() {
        for (auto& e: occupancy_ser)
            e=0;
        for (auto& e: occupancy_buf)
            e=0;
        for (auto& e: rawWaitingTime)
            e.clear();
        for (auto& e: rawResponseTime)
            e.clear();
        for (auto& e: completion)
            e=0;
        for (auto& e: curr_job_seq)
            e=0;
        for (auto& e: tot_job_seq)
            e=0;
        for (auto& e: curr_job_seq_start)
            e-=simtime;
        for (auto& e: tot_job_seq_dur)
            e=0;
        for (auto& e: job_seq_amount)
            e=0;
        for (auto& e: preemption)
            e=0;
        for (auto& e: rep_free_servers_distro)
            e=0;
        for (auto& e: fel)
            e-=simtime;
        
        if (this->w == -2 || this->sampling_method != 10) {
            for (int i = 0; i < jobs_inservice.size(); ++i) {
                for (auto job = jobs_inservice[i].begin(); job != jobs_inservice[i].end(); ++job) {
                    jobs_inservice[i][job->first] -= simtime;
                }
            }

            for (auto job_id = arrTime.begin(); job_id != arrTime.end(); ++job_id) {
                arrTime[job_id->first] -= simtime;
            }
        }

        phase_two_start -= simtime;
        phase_three_start -= simtime;

        policy->reset_completion(simtime);
        
        simtime = 0.0;
        util = 0.0;
        waste = 0.0;
        viol = 0.0;
        occ = 0.0;
        windowSize.clear();
        phase_two_duration = 0;
        phase_three_duration = 0;
        
    }

      
    void simulate(unsigned long nevents, int repetitions=1) {
        rep_th.clear();
        rep_occupancy_buf.clear();
        rep_occupancy_ser.clear();
        rep_wait.clear();
        rep_wait_var.clear();
        rep_resp.clear();
        rep_resp_var.clear();
        rep_waste.clear();
        rep_viol.clear();
        rep_util.clear();
        rep_tot_buf.clear();
        rep_tot_wait.clear();
        rep_tot_wait_var.clear();
        rep_tot_resp.clear();
        rep_timings.clear();

        rep_big_seq_avg_len.clear();
        rep_small_seq_avg_len.clear();
        rep_big_seq_avg_dur.clear();
        rep_small_seq_avg_dur.clear();
        rep_big_seq_amount.clear();
        rep_small_seq_amount.clear();
        rep_big_seq_max_len.clear();
        rep_small_seq_max_len.clear();
        rep_big_seq_max_dur.clear();
        rep_small_seq_max_dur.clear();

        rep_phase_two_duration.clear();
        rep_phase_three_duration.clear();

        rep_window_size.clear();
        rep_preemption.clear();

        
        // double tot_lambda = std::accumulate(l.begin(), l.end(), 0.0);
        // std::string out_filename = "logfile_N" + std::to_string(n) + "_" + std::to_string(tot_lambda) + "_W" + std::to_string(w) + ".csv";
        // remove(out_filename.c_str());
        // std::ofstream outputFile_rep(out_filename, std::ios::app);
        // std::vector<std::string> headers_rep;
        // headers_rep = {"Repetition"};
        // for (int ts : sizes) {
        //     headers_rep.push_back("T" + std::to_string(ts) + " Queue");
        //     headers_rep.push_back("T" + std::to_string(ts) + " MQL");
        // }
        //
        // headers_rep.push_back("Total Queue");
        // headers_rep.push_back("Total MQL");
        //
        // if (outputFile_rep.tellp() == 0) {
        //     // Write the headers to the CSV file
        //     for (const std::string& header : headers_rep) {
        //         outputFile_rep << header << ";";
        //     }
        //     outputFile_rep << "\n";
        // }
        // outputFile_rep.close();
        // std::vector<std::string> headers;
        // headers = {"Repetition","Event"};
        // for (int ts : sizes) {
        //     headers.push_back("T" + std::to_string(ts) + " Queue");
        //     headers.push_back("T" + std::to_string(ts) + " Service");
        // }
        //
        // headers.push_back("Total Queue");
        // headers.push_back("Total Service");
        //
        // for (int i = 0; i < sizes.size(); ++i) {
        //     headers.push_back("Fel" + std::to_string(i));
        // }
        //
        // for (int i = 0; i < sizes.size(); ++i) {
        //     headers.push_back("Fel" + std::to_string(i+sizes.size()));
        // }
        //
        // headers.push_back("Simtime");
        //
        // remove(logfile_name.c_str());
        // std::ofstream outputFile(logfile_name, std::ios::app);
        //
        //
        // if (outputFile.tellp() == 0) {
        //     // Write the headers to the CSV file
        //     for (const std::string& header : headers) {
        //         outputFile << header << ";";
        //     }
        //     outputFile << "\n";
        // }
        // outputFile.close();

        for (int rep=0; rep<repetitions; rep++) {

            /*int buf_size = std::reduce(policy->get_state_buf().begin(), policy->get_state_buf().end());
            if (buf_size > 180000000) {
                break;
            }*/

        	double current_seq_len = 0.0;
            double tot_seq_len = 0.0;
            double seq_amount = 0.0;
            double max_seq_len = 0.0;

        	auto start = std::chrono::high_resolution_clock::now();
            reset_statistics();
            auto last_dep = std::chrono::steady_clock::now();

            for (unsigned long int k=0; k<nevents; k++) {
                int big_state ;
                auto itmin = std::min_element(fel.begin(), fel.end());
                //std::cout << *itmin << std::endl;
                int pos = static_cast<int>(itmin-fel.begin());
                //std::cout << pos << std::endl;
                collect_statistics(pos);
                //std::cout << "collect" << std::endl;
                if (pos<nclasses) { //departure
                    last_dep = std::chrono::steady_clock::now();
                    if (this->w == -2 || this->sampling_method != 10) {
                        jobs_inservice[pos].erase(job_fel[pos]); // Remove jobs from in_service (they cannot be in preempted list)
                        rawWaitingTime[pos].push_back(waitTime[job_fel[pos]]);
                        rawResponseTime[pos].push_back(waitTime[job_fel[pos]]+holdTime[job_fel[pos]]);
                        arrTime.erase(job_fel[pos]);
                        waitTime.erase(job_fel[pos]);
                        holdTime.erase(job_fel[pos]);
                    }
                    //std::cout << "before dep" << std::endl;

                    policy->departure(pos,sizes[pos],job_fel[pos]);
                    //std::cout << "dep" << std::endl;
                }
                else {
                    auto job_id = k+(nevents*rep);
                    if (this->w == -3) {
                        holdTime[job_id] = sample_st(1/u[pos-nclasses]);
                        //std::cout << holdTime[job_id] << std::endl;
                    }
                    policy->arrival(pos-nclasses,sizes[pos-nclasses], job_id);
                    arrTime[job_id] = *itmin;
                    //std::cout << "arr" << std::endl;

                }
                
                simtime = *itmin;
                
                resample();
                //std::cout << "resample" << std::endl;
                if (this->w == -3) {
                    //std::cout << policy->get_state_buf()[0] << " " << simtime << std::endl;
                    bool added;
                    added = policy->fit_jobs(holdTime, simtime);
                    resample();
                    int idx = 0;
                    while (added) {
                        //std::cout << idx << " " << simtime << std::endl;
                        added = policy->fit_jobs(holdTime, simtime);
                        //std::cout << "added" << std::endl;
                        resample();
                        //std::cout << "resample" << std::endl;
                        idx += 1;
                    }
                    //std::cout << "out" << std::endl;
                }
                /*if (k % 1000 == 0) {
                    std::ofstream outputFile(logfile_name, std::ios::out);
                    for (const std::string& header : headers) {
                        outputFile << header << ";";
                    }
                    outputFile << "\n";
                    outputFile << rep << ";";
                    outputFile << k << ";";
                    auto state_buf = policy->get_state_buf();
                    auto state_ser = policy->get_state_ser();
                    for (int i=0; i<occupancy_buf.size(); i++) {
                        outputFile << state_buf[i] << ";";
                        outputFile << state_ser[i] << ";";
                    }
                    outputFile << std::accumulate(state_buf.begin(), state_buf.end(), 0.0) << ";";
                    outputFile << std::accumulate(state_ser.begin(), state_ser.end(), 0.0) << ";";
                    for (int i=0; i<fel.size(); i++) {
                        outputFile << fel[i] << ";";
                    }
                    outputFile << simtime << ";";
                    outputFile << "\n";
                    outputFile.close();
                    //std::cout << "logs" << std::endl;
                }*/ /*else if ( std::chrono::steady_clock::now() - last_dep >= std::chrono::minutes(1) ) {
                    std::ofstream outputFile(logfile_name, std::ios::out);
                    for (const std::string& header : headers) {
                        outputFile << header << ";";
                    }
                    outputFile << "\n";
                    outputFile << rep << ";";
                    outputFile << k << ";";
                    auto state_buf = policy->get_state_buf();
                    auto state_ser = policy->get_state_ser();
                    for (int i=0; i<occupancy_buf.size(); i++) {
                        outputFile << state_buf[i] << ";";
                        outputFile << state_ser[i] << ";";
                    }
                    outputFile << std::accumulate(state_buf.begin(), state_buf.end(), 0.0) << ";";
                    outputFile << std::accumulate(state_ser.begin(), state_ser.end(), 0.0) << ";";
                    outputFile << "\n";
                    outputFile.close();
                    last_dep = std::chrono::steady_clock::now();
                }*/
            }

            double avg_seq_len = (tot_seq_len*1.0)/seq_amount;
            
            double totq = 0.0;
            for (auto& x: occupancy_buf) {
                x /= simtime;
                totq += x;
            }


            rep_occupancy_buf.push_back(occupancy_buf);
            
            double tots = 0.0;
            for (auto& x: occupancy_ser) {
                x /= simtime;
                tots += x;
            }
            rep_occupancy_ser.push_back(occupancy_ser);
            
            for (int i=0; i<completion.size(); i++)
                throughput[i]=completion[i]/simtime;
            rep_th.push_back(throughput);

            double totx = 0.0;
            std::list<double> totRawWaitingTime;
            std::list<double> totRawResponseTime;
            std::vector<double> preemption_avg;
            preemption_avg.resize(sizes.size());
            for (int i=0; i< occupancy_buf.size(); i++) {
                //waitingTime[i] = occupancy_buf[i] / throughput[i];
                double mean_wt = std::accumulate(rawWaitingTime[i].begin(), rawWaitingTime[i].end(), 0.0)/rawWaitingTime[i].size();
                waitingTime[i] = mean_wt;

                double tot_diff = 0.0;
                for (auto& rawWt: rawWaitingTime[i]){
                    tot_diff += pow(rawWt-mean_wt,2);
                }
                waitingTimeVar[i] = tot_diff/rawWaitingTime[i].size();

                //responseTime[i] = (occupancy_buf[i]+occupancy_ser[i]) / throughput[i];
                double mean_rt = std::accumulate(rawResponseTime[i].begin(), rawResponseTime[i].end(), 0.0)/rawResponseTime[i].size();
                responseTime[i] = mean_rt;

                tot_diff = 0.0;
                for (auto& rawRt: rawResponseTime[i]){
                    tot_diff += pow(rawRt-mean_rt,2);
                }
                responseTimeVar[i] = tot_diff/rawResponseTime[i].size();

                totx += throughput[i];
                totRawWaitingTime.insert(totRawWaitingTime.end(), rawWaitingTime[i].begin(), rawWaitingTime[i].end());
                totRawResponseTime.insert(totRawResponseTime.end(), rawResponseTime[i].begin(), rawResponseTime[i].end());

                preemption_avg[i] = ((double)preemption[i])/(double)completion[i];
            }

            for (int i=0; i<occupancy_ser.size(); i++)
                util += occupancy_ser[i]*sizes[i];

            rep_window_size.push_back(std::accumulate(windowSize.begin(), windowSize.end(), 0.0)/simtime);
            rep_preemption.push_back(preemption_avg);

            rep_wait.push_back(waitingTime);
            rep_wait_var.push_back(waitingTimeVar);
            rep_resp.push_back(responseTime);
            rep_resp_var.push_back(responseTimeVar);
            rep_waste.push_back(waste/simtime);
            rep_viol.push_back(viol/simtime);
            rep_util.push_back(util/n);
            rep_tot_buf.push_back(totq);
            //rep_tot_wait.push_back(totq/totx);
            double mean_wt = std::accumulate(totRawWaitingTime.begin(), totRawWaitingTime.end(), 0.0)/totRawWaitingTime.size();
            rep_tot_wait.push_back(mean_wt);

            double tot_diff = 0.0;
            for (auto& rawWt: totRawWaitingTime){
                tot_diff += pow(rawWt-mean_wt,2);
            }
            rep_tot_wait_var.push_back(tot_diff/totRawWaitingTime.size());

            //rep_tot_resp.push_back((totq+tots)/totx);
            double mean_rt = std::accumulate(totRawResponseTime.begin(), totRawResponseTime.end(), 0.0)/totRawResponseTime.size();
            rep_tot_resp.push_back(mean_rt);

            tot_diff = 0.0;
            for (auto& rawRt: totRawResponseTime){
                tot_diff += pow(rawRt-mean_rt,2);
            }
            rep_tot_resp_var.push_back(tot_diff/totRawResponseTime.size());

            //std::cout<<tot_job_seq_dur[0]<<std::endl;
            //std::cout<<job_seq_amount[0]<<std::endl;

            double avg_big_seq_len = (tot_job_seq[1]*1.0)/job_seq_amount[1];
            double avg_small_seq_len = (tot_job_seq[0]*1.0)/job_seq_amount[0];
            double avg_big_seq_dur = (tot_job_seq_dur[1]*1.0)/job_seq_amount[1];
            double avg_small_seq_dur = (tot_job_seq_dur[0]*1.0)/job_seq_amount[0];

            rep_big_seq_avg_len.push_back(avg_big_seq_len);
            rep_small_seq_avg_len.push_back(avg_small_seq_len);
            rep_big_seq_avg_dur.push_back(avg_big_seq_dur);
            rep_small_seq_avg_dur.push_back(avg_small_seq_dur);
            rep_big_seq_amount.push_back(job_seq_amount[1]);
            rep_small_seq_amount.push_back(job_seq_amount[0]);

            rep_phase_two_duration.push_back((phase_two_duration*1.0)/job_seq_amount[0]);
            rep_phase_three_duration.push_back((phase_three_duration*1.0)/job_seq_amount[0]);

            /*std::cout << phase_two_duration << std::endl;
            std::cout << phase_three_duration << std::endl;
            std::cout << phase_two_duration+phase_three_duration << std::endl;
            std::cout << tot_job_seq_dur[0] << std::endl;                
            std::cout << "-------------------------------------" << std::endl;*/
            
            //rep_big_sequences.push_back(avg_seq_len);
            //rep_seq_amount.push_back(seq_amount);
            //rep_seq_max_len.push_back(max_seq_len);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

            rep_timings.push_back(duration);
            std::cout << "Repetition " << std::to_string(rep) << " Done" << std::endl;
            //this->reset_data();
            /*auto sb = policy->get_state_buf();
            for (int i = 0; i<sb.size(); ++i) {
                std::cout << sb[i] << ", ";
            }
            std::cout << std::endl;*/
            // std::ofstream outputFile(out_filename, std::ios::app);
            // outputFile << rep << ";";
            // auto state_buf = policy->get_state_buf();
            // for (int i=0; i<occupancy_buf.size(); i++) {
            //     outputFile << state_buf[i] << ";";
            //     outputFile << occupancy_buf[i] << ";";
            // }
            // outputFile << std::accumulate(state_buf.begin(), state_buf.end(), 0.0) << ";";
            // outputFile << std::accumulate(occupancy_buf.begin(), occupancy_buf.end(), 0.0) << ";";
            // outputFile << "\n";
            // outputFile.close();
        }

        //outputFile.close();
        for (auto& x: rep_free_servers_distro) {
            x /= simtime;
        }

    /*    std::ofstream outFree("freeserversDistro-nClasses" + std::to_string(this->nclasses) + "-N" + std::to_string(this->n) + "-Win" + std::to_string(this->w) + ".csv", std::ios::app);
        lambda = std::accumulate(this->l.begin(), this->l.end(), 0.0);

        if (outFree.tellp() == 0) {
            // Write the headers to the CSV file
            outFree << "Arrival Rate" << ";";
            for (size_t i = 0; i < rep_free_servers_distro.size(); ++i) {
                outFree << i << ";";
            }
            outFree << "\n";
        }
        outFree << lambda << ";";
        
        for (int i = 0; i <= rep_free_servers_distro.size(); i++) {

                outFree << rep_free_servers_distro[i] << ";";

        }
        outFree << "\n";
        outFree.close();*/
    }

    void produce_statistics(std::vector<Confidence_inter>& occ_buff, std::vector<Confidence_inter>& occ_ser, std::vector<Confidence_inter>& throughput, 
                            std::vector<Confidence_inter>& waitTime, std::vector<Confidence_inter>& waitTimeVar, std::vector<Confidence_inter>& respTime, std::vector<Confidence_inter>& respTimeVar,
                            std::vector<bool>& warnings, std::vector<Confidence_inter>& preemption_avg, Confidence_inter &wasted, Confidence_inter &violations, Confidence_inter &utilisation,
                            Confidence_inter &occ_tot, Confidence_inter &wait_tot, Confidence_inter &wait_var_tot, Confidence_inter &resp_tot, Confidence_inter &resp_var_tot, Confidence_inter &timings_tot, 
                            Confidence_inter &big_seq_len, Confidence_inter &small_seq_len, Confidence_inter &big_seq_dur, Confidence_inter &small_seq_dur, Confidence_inter &big_seq_amount, Confidence_inter &small_seq_amount,
                            Confidence_inter &phase_two_dur, Confidence_inter &phase_three_dur, Confidence_inter &windowSize, double confidence = 0.05) {
        for (int i=0; i<nclasses; i++) {
            occ_buff.push_back(compute_class_interval(rep_occupancy_buf, i, confidence));
            occ_ser.push_back(compute_class_interval(rep_occupancy_ser, i, confidence));
            throughput.push_back(compute_class_interval(rep_th, i, confidence));
            waitTime.push_back(compute_class_interval(rep_wait, i, confidence));
            waitTimeVar.push_back(compute_class_interval(rep_wait_var, i, confidence));
            respTime.push_back(compute_class_interval(rep_resp, i, confidence));
            respTimeVar.push_back(compute_class_interval(rep_resp_var, i, confidence));
            preemption_avg.push_back(compute_class_interval(rep_preemption, i, confidence));

            if (1.0-throughput[throughput.size()-1].mean/l[i]>0.05)
                warnings.push_back(true);
            else
                warnings.push_back(false);
        }
        wasted = compute_interval(rep_waste, confidence);
        violations = compute_interval(rep_viol, confidence);
        utilisation = compute_interval(rep_util, confidence);
        occ_tot = compute_interval(rep_tot_buf, confidence);
        wait_tot = compute_interval(rep_tot_wait, confidence);
        wait_var_tot = compute_interval(rep_tot_wait_var, confidence);
        resp_tot = compute_interval(rep_tot_resp, confidence);
        resp_var_tot = compute_interval(rep_tot_resp_var, confidence);
        timings_tot = compute_interval(rep_timings, confidence);
        big_seq_len = compute_interval(rep_big_seq_avg_len, confidence);
        small_seq_len = compute_interval(rep_small_seq_avg_len, confidence);
        big_seq_dur = compute_interval(rep_big_seq_avg_dur, confidence);
        small_seq_dur = compute_interval(rep_small_seq_avg_dur, confidence);
        big_seq_amount = compute_interval(rep_big_seq_amount, confidence);
        small_seq_amount = compute_interval(rep_small_seq_amount, confidence);
        phase_two_dur = compute_interval(rep_phase_two_duration, confidence);
        phase_three_dur = compute_interval(rep_phase_three_duration, confidence);
        windowSize = compute_interval(rep_window_size, confidence);
    }
            
    
private:
    std::vector<double> l;
    std::vector<double> u;
    std::vector<int> sizes;
    int n;
    int w;
    int nclasses;
    bool debugMode;
    
    Policy* policy;
    
    double simtime = 0.0;
    
    //overall statistics
    std::vector<std::vector<double>> rep_occupancy_buf;
    std::vector<std::vector<double>> rep_occupancy_ser;
    std::vector<double> rep_free_servers_distro;
    std::vector<std::vector<double>> rep_th;
    std::vector<std::vector<double>> rep_wait;
    std::vector<std::vector<double>> rep_wait_var;
    std::vector<std::vector<double>> rep_resp;
    std::vector<std::vector<double>> rep_resp_var;
    std::vector<double> rep_tot_wait;
    std::vector<double> rep_tot_wait_var;
    std::vector<double> rep_tot_resp;
    std::vector<double> rep_tot_resp_var;
    std::vector<double> rep_timings;
    std::vector<double> rep_tot_buf;
    std::vector<double> rep_waste;
    std::vector<double> rep_viol;
    std::vector<double> rep_util;

    std::vector<double> rep_big_seq_avg_len;
    std::vector<double> rep_small_seq_avg_len;
    std::vector<double> rep_big_seq_avg_dur;
    std::vector<double> rep_small_seq_avg_dur;
    std::vector<double> rep_big_seq_amount;
    std::vector<double> rep_small_seq_amount;
    std::vector<double> rep_big_seq_max_len;
    std::vector<double> rep_small_seq_max_len;
    std::vector<double> rep_big_seq_max_dur;
    std::vector<double> rep_small_seq_max_dur;

    std::vector<double> rep_window_size;
    std::vector<std::vector<double>> rep_preemption;

    std::vector<double> rep_phase_two_duration;
    std::vector<double> rep_phase_three_duration;

    //statistics for single run
    std::vector<double> occupancy_buf;
    std::vector<double> occupancy_ser;
    std::vector<unsigned long> completion;
    std::vector<unsigned long> preemption;
    std::vector<double> throughput;
    std::vector<double> waitingTime;
    std::vector<double> waitingTimeVar;

    std::unordered_map<long int,double> arrTime;
    std::unordered_map<long int,double> waitTime;
    std::unordered_map<long int,double> holdTime;
    std::vector<std::list<double>> rawWaitingTime;
    std::vector<std::list<double>> rawResponseTime;

    std::list<double> windowSize;

    std::vector<double> responseTime;
    std::vector<double> responseTimeVar;

    double lambda;
    double waste = 0.0;
    double viol = 0.0;
    double util = 0.0;
    double occ = 0.0;

    std::vector<double> curr_job_seq;
    std::vector<double> curr_job_seq_start;
    std::vector<double> tot_job_seq;
    std::vector<double> tot_job_seq_dur;
    std::vector<double> job_seq_amount;
    int last_job = -1;
    double phase_two_duration = 0;
    double phase_three_duration = 0;
    double phase_two_start = 0;
    double phase_three_start = 0;
    int curr_phase;
    bool add_phase_two = false;
    
    std::vector<double> fel;
    std::vector<long int> job_fel;
    //std::vector<std::list<double>> fels;
    //std::list<int> job_ids;
    std::vector<std::unordered_map<long int,double>> jobs_inservice; //[id, time_end]
    std::vector<std::unordered_map<long int,double>> jobs_preempted; //[id, time_left]

    std::string logfile_name;
    
    std::mt19937_64* generator;
    std::uniform_real_distribution<double> ru;

    int sampling_method;
    double alfa = 2;
    double mean_ratio = (alfa-1)/alfa;
    
    double sample_exp(double par) {
        return -log(ru(*generator))/par;
    }

    double sample_pareto(double xm) {
        return xm*exp(sample_exp(alfa));
    }

    /*double sample_pareto_v2(double xm) {
        double p = ru(*generator);
        return xm*pow(1-p,-(1/alfa));
    }

    double sample_bounded_pareto(double min, double max) {
        double p = ru(*generator);
        double num = pow(min,alfa);
        double den = 1-(p*(1-pow(min/max,alfa)));
        return pow(num/den,1/alfa);
    }*/

    double sample_bPareto(double par) {
        double l = (12000.0/23999.0)*par;
        double h =  12000*par;
        double u = ru(*generator);
        double num = (u*pow(h, alfa)) - (u*pow(l, alfa)) - pow(h, alfa);
        double den = pow(h, alfa)*pow(l, alfa);
        double frac = num/den; 
        return pow(-frac, -1/alfa);
    }

    double sample_unif(double par) {
        std::uniform_real_distribution<double> ru(0.5*par, 1.5*par);
        return ru(*generator);
    }

    double frec_alfa = 2.15;
    double s_ratio = 1/(std::tgammaf(1-(1/frec_alfa)));

    double sample_frechet(double par) {
        //std::cout << s_ratio << std::endl;
        return (s_ratio/par)*pow((-log(ru(*generator))),-1/frec_alfa);
    }

    double sample_st(double rate) {
        if (this->sampling_method == 0) {
            return sample_exp(rate);
        } else if (this->sampling_method == 1) {
            //return (mean_ratio*(1/rate))*exp(sample_exp(alfa));
            //return sample_bPareto(1/rate);
            return sample_pareto(mean_ratio*(1/rate));
            //return sample_bounded_pareto(0.5*(1/rate),2*(1/rate));
        } else if (this->sampling_method == 2) {
            return 1/rate;
        } else if (this->sampling_method == 3) {
            return sample_unif(1/rate);
        } else if (this->sampling_method == 4) {
            return sample_bPareto(1/rate);
            //return sample_bounded_pareto_v2((120000.0/239999.0)*(1/rate),120000*(1/rate));
        } else if (this->sampling_method == 5) {
            return sample_frechet(rate);
        }
        return 0;
    }
    
    void resample() {
        //add arrivals and departures
        if (this->w == -2) { //special blocks for serverFilling (memoryful)
            auto stopped_jobs = policy->get_stopped_jobs();
            auto ongoing_jobs = policy->get_ongoing_jobs();
            bool stopped;
            for (int i=0; i<nclasses; i++) {
                if (fel[i+nclasses] <= simtime) { // only update arrival that is executed at the time
                    fel[i+nclasses] = sample_exp(l[i]) + simtime;
                }

                for (auto job_id = stopped_jobs[i].begin(); job_id != stopped_jobs[i].end(); ++job_id) {
                    if (jobs_inservice[i].find(*job_id) != jobs_inservice[i].end()) { // If they are currently being served: stop them
                        jobs_preempted[i][*job_id] = jobs_inservice[i][*job_id]-simtime; // Save the remaining service time
                        jobs_inservice[i].erase(*job_id);
                        arrTime[*job_id] = simtime;
                        preemption[i]++;
                    }
                }

                long int fastest_job_id;
                double fastest_job_fel = std::numeric_limits<double>::infinity();
                for (auto job_id = ongoing_jobs[i].begin(); job_id != ongoing_jobs[i].end(); ++job_id) {
                    if (jobs_inservice[i].find(*job_id) == jobs_inservice[i].end()) { // If they are NOT already in service
                        if (jobs_preempted[i].find(*job_id) != jobs_preempted[i].end()) { // See if they were preempted: resume them
                            jobs_inservice[i][*job_id] = jobs_preempted[i][*job_id]+simtime;
                            jobs_preempted[i].erase(*job_id);
                            waitTime[*job_id] = simtime-arrTime[*job_id]+waitTime[*job_id];
                        } else { // or they are just new jobs about to be served for the first time: add them with new service time
                            double sampled = sample_st(1/u[i]);
                            jobs_inservice[i][*job_id] = sampled + simtime;
                            //rawWaitingTime[i].push_back(simtime-arrTime[*job_id]);
                            //arrTime.erase(*job_id); //update waitingTime
                            waitTime[*job_id] = simtime-arrTime[*job_id];
                            holdTime[*job_id] = sampled;
                        }

                        if (jobs_inservice[i][*job_id] < fastest_job_fel) {
                            fastest_job_id = *job_id;
                            fastest_job_fel = jobs_inservice[i][*job_id];
                        }
                    } else { // They are already in service
                        if (jobs_inservice[i][*job_id] < fastest_job_fel) {
                            fastest_job_id = *job_id;
                            fastest_job_fel = jobs_inservice[i][*job_id];
                        }
                    }
                }

                if (jobs_inservice[i].empty()) { // If no jobs in service for a given class
                    fel[i] = std::numeric_limits<double>::infinity();
                } else {
                    fel[i] = fastest_job_fel;
                    job_fel[i] = fastest_job_id;
                }
            }
        } else if (this->sampling_method != 10) { //exponential distro can use the faster memoryless blocks
            auto ongoing_jobs = policy->get_ongoing_jobs();
            int pooled_i;
            for (int i=0; i<nclasses; i++) {
                if (i < nclasses-1) {
                    pooled_i = 0;
                } else {
                    pooled_i = 1;
                }

                if (fel[i+nclasses] <= simtime) { // only update arrival that is executed at the time
                    fel[i+nclasses] = sample_exp(l[i]) + simtime;
                }

                //std::cout << ongoing_jobs[i].size() << std::endl;
                for (long int job_id : ongoing_jobs[i]) {
                    double sampled;
                    if (this->w == -3) {
                        sampled = holdTime[job_id];
                        policy->insert_completion(this->sizes[i],sampled + simtime);
                    } else {
                        sampled = sample_st(1/u[i]);
                    }
                    jobs_inservice[i][job_id] = sampled + simtime;
                    //rawWaitingTime[i].push_back();
                    waitTime[job_id] = simtime-arrTime[job_id];
                    holdTime[job_id] = sampled;
                    //arrTime.erase(job_id); //update waitingTime
                    if (jobs_inservice[i][job_id] < fel[i]) {
                        fel[i] = jobs_inservice[i][job_id];
                        job_fel[i] = job_id;
                    }

                    if (last_job < 0) {
                        //std::cout << "sequence " << i << " starting from idle" << " simtime " << simtime << std::endl;
                        curr_job_seq[pooled_i] = 1;
                        curr_job_seq_start[pooled_i] = simtime;
                        last_job = pooled_i;
                        if (pooled_i == 0) {
                            //std::cout << "phase three starting from idle" << " simtime " << simtime << std::endl;
                            phase_three_start = simtime;
                            curr_phase = 3;
                        }
                        //std::cout << "-------------------------------------" << std::endl;
                    } else if (last_job == pooled_i) {
                        curr_job_seq[pooled_i] = curr_job_seq[pooled_i] + 1; 
                    } else {
                        //std::cout << "sequence " << pooled_i << " starting from sequence " << last_job << " simtime " << simtime << std::endl;
                        tot_job_seq[last_job] = tot_job_seq[last_job]+curr_job_seq[last_job];
                        //if (last_job == 1 && pooled_i == 0) {
                        //    std::cout << simtime << "  " << curr_job_seq_start[last_job] << std::endl;
                        //}
                        tot_job_seq_dur[last_job] = tot_job_seq_dur[last_job] + simtime - curr_job_seq_start[last_job];
                        job_seq_amount[last_job] = job_seq_amount[last_job] + 1;
                        curr_job_seq[last_job] = 0;
                        curr_job_seq[pooled_i] = 1;
                        curr_job_seq_start[pooled_i] = simtime;
                        //std::cout << tot_job_seq_dur[last_job] << std::endl;
                        last_job = pooled_i;
                        if (pooled_i == 0) {
                            if (policy->get_free_ser() == 0){
                                //std::cout << "phase two starting from phase one" << " simtime " << simtime << std::endl;
                                phase_two_start = simtime;
                                curr_phase = 2;
                            } else {
                                //std::cout << "phase three starting from phase one" << " simtime " << simtime << std::endl;
                                phase_three_start = simtime;
                                curr_phase = 3;
                            }
                            //std::cout << "-------------------------------------" << std::endl;
                        }
                        else {
                            //std::cout << "phase one starting from phase three" << " simtime " << simtime << std::endl;
                            phase_three_duration += (simtime-phase_three_start);
                            if (add_phase_two) {
                                phase_two_duration += (phase_three_start-phase_two_start);
                                add_phase_two = false;
                            }
                            //std::cout << phase_two_duration+phase_three_duration << std::endl;
                            //std::cout << tot_job_seq_dur[0] << std::endl; 
                            //std::cout << (phase_three_duration+phase_two_duration == tot_job_seq_dur[0]) << std::endl; 
                            if (phase_two_start < 0 && phase_two_duration == 0 && phase_three_duration < tot_job_seq_dur[0]) {
                                //phase_two_duration += (phase_three_start-phase_two_start);
                            }
                            curr_phase = 1;
                            //std::cout << "small abis" << std::endl;
                            //std::cout << policy->get_state_ser()[0] << " " << policy->get_state_buf()[1] << std::endl;
                            //std::cout << phase_three_start << std::endl;
                            //std::cout << curr_job_seq_start[0] << std::endl;
                            //std::cout << phase_two_start << std::endl;
                            //std::cout << phase_three_start << std::endl;
                            //std::cout << phase_two_duration << std::endl;
                            //std::cout << phase_three_duration << std::endl;
                            //std::cout << phase_two_duration+phase_three_duration << std::endl;
                            //std::cout << tot_job_seq_dur[0] << std::endl;                
                            //std::cout << "-------------------------------------" << std::endl;
                        }
                    }
                }

                if (jobs_inservice[i].empty()) { // If no jobs in service for a given class
                    fel[i] = std::numeric_limits<double>::infinity();
                } else if (fel[i] <= simtime) {
                    fel[i] = std::numeric_limits<double>::infinity();
                    for (auto& job : jobs_inservice[i]) {
                        if (job.second < fel[i]) {
                            fel[i] = job.second;
                            job_fel[i] = job.first;
                        }
                    }
                }
            }

            if (curr_phase == 2) {
                if (this->w > -4 && policy->get_free_ser() > 0){
                    //std::cout << "phase three starting from phase two" << " simtime " << simtime << std::endl;
                    //phase_two_duration += (simtime-phase_two_start);
                    phase_three_start = simtime;
                    curr_phase = 3;
                    add_phase_two = true;
                    //std::cout << phase_two_duration << std::endl;
                    //std::cout << "-------------------------------------" << std::endl;
                } else if (this->w <= -4 && policy->prio_big() == true){
                    //std::cout << "phase three starting from phase two" << " simtime " << simtime << std::endl;
                    //phase_two_duration += (simtime-phase_two_start);
                    phase_three_start = simtime;
                    curr_phase = 3;
                    add_phase_two = true;
                    //std::cout << phase_two_duration << std::endl;
                    //std::cout << "-------------------------------------" << std::endl;
                }
                
            }

            if (policy->get_free_ser() == this->n && last_job >= 0) {
                //std::cout << "sequence " << last_job << " ending" << " simtime " << simtime << std::endl;
                tot_job_seq[last_job] = tot_job_seq[last_job]+curr_job_seq[last_job];
                tot_job_seq_dur[last_job] = tot_job_seq_dur[last_job] + simtime - curr_job_seq_start[last_job];
                job_seq_amount[last_job] = job_seq_amount[last_job] + 1;
                curr_job_seq[last_job] = 0;
                //std::cout << tot_job_seq_dur[last_job] << std::endl;
                if (last_job == 0) {
                    //std::cout << "phase three ending" << " simtime " << simtime << std::endl;
                    //std::cout << add_phase_two << std::endl;
                    //std::cout << phase_two_start << std::endl;
                    //std::cout << phase_three_start << std::endl;
                    if (curr_phase == 3) {
                        phase_three_duration += (simtime-phase_three_start);
                        if (add_phase_two) {
                            phase_two_duration += (phase_three_start-phase_two_start);
                            add_phase_two = false;
                        }
                        if (phase_two_start < 0 && phase_two_duration == 0 && phase_two_duration+phase_three_duration < tot_job_seq_dur[0]) {
                            //std::cout << "HEHE-------------------------------------" << std::endl;
                            //phase_two_duration += (phase_three_start-phase_two_start);
                        }
                    } else if (curr_phase == 2) {
                        phase_two_duration += (simtime-phase_two_start);
                    } else {
                        std::cout << "WADAW-------------------------------------" << std::endl;
                    }
                    //std::cout << phase_two_duration << std::endl;
                    //std::cout << phase_three_duration << std::endl;
                    //std::cout << phase_two_duration+phase_three_duration << std::endl;
                    //std::cout << tot_job_seq_dur[0] << std::endl; 
                    //std::cout << phase_three_duration << std::endl;
                    //std::cout << "-------------------------------------" << std::endl;
                }
                last_job = -1;
                curr_phase = 0;
                //std::cout << "-------------------------------------" << std::endl;
            }

            if (curr_phase == 3) {
                //std::cout << policy->get_state_ser()[0] << " " << policy->get_state_buf()[1] << std::endl;
            }

        } else {
            for (int i=0; i<nclasses; i++) {
                fel[i+nclasses] = sample_exp(l[i]) + simtime;
                if (policy->get_state_ser()[i] > 0) {
                    fel[i] = sample_exp((1/u[i])*policy->get_state_ser()[i]) + simtime;
                    //fel[i] = sample_st((1/u[i])*policy->get_state_ser()[i]) + simtime;
                } else {
                    fel[i] = std::numeric_limits<double>::infinity();
                }
            }
        }

    }
    
    void collect_statistics(int pos) {
        
        if (pos<nclasses)
            completion[pos]++;
        
        double delta = fel[pos]-simtime;
        for (int i=0; i<nclasses; i++) {
            occupancy_buf[i] += policy->get_state_buf()[i]*delta;
            occupancy_ser[i] += policy->get_state_ser()[i]*delta;
        }
        auto occ = 0;
        for (int i=0; i<nclasses; i++) {
            occ += policy->get_state_ser()[i]*sizes[i];
        }
        waste += (n-occ)*delta;
        viol += policy->get_violations_counter()*delta;
        rep_free_servers_distro[policy->get_free_ser()] += delta;

        windowSize.push_back(policy->get_window_size()*delta);
    }
    
    Confidence_inter compute_class_interval(const std::vector<std::vector<double>>& rep, int cl, double confidence) {
        
        double mean = 0.0;
        double stdv = 0.0;
        
        for (int i=0; i<rep.size(); i++)
            mean+=rep[i][cl];
        mean/=rep.size();
        
        for (int i=0; i<rep.size(); i++)
            stdv+=std::pow(rep[i][cl]-mean, 2);
        
        stdv = std::sqrt(stdv);
        if (rep.size()>1)
            stdv/=(rep.size()-1);
        else
            stdv = 0.0;
        
        boost::math::students_t dist(rep.size()-1);
        double t = boost::math::quantile(boost::math::complement(dist, confidence/2.0));
        double delta = t*stdv / std::sqrt(static_cast<double>(rep.size()));
        
        return Confidence_inter{mean-delta, mean+delta, mean};
        
    }

    /*Confidence_inter compute_class_interval_chi(const std::vector<std::vector<double>>& rep, int cl, double confidence) {
        
        double mean = 0.0;
        double stdv = 0.0;
        
        for (int i=0; i<rep.size(); i++)
            mean+=rep[i][cl];
        mean/=rep.size();
        
        for (int i=0; i<rep.size(); i++)
            stdv+=std::pow(rep[i][cl]-mean, 2);
        
        stdv = std::sqrt(stdv);
        if (rep.size()>1)
            stdv/=(rep.size()-1);
        else
            stdv = 0.0;
        
        boost::math::chi_squared dist(rep.size()-1);
        double t_right = boost::math::quantile(boost::math::complement(dist, confidence/2.0));
        double t_left = boost::math::quantile(boost::math::complement(dist, (1.0-confidence)/2.0));
        double delta_right = t_right*stdv / std::sqrt(static_cast<double>(rep.size()));
        
        return Confidence_inter{mean-delta, mean+delta, mean};
        
    }*/

    Confidence_inter compute_interval(const std::vector<double>& rep, double confidence) {

        double mean = 0.0;
        double stdv = 0.0;

        for (int i=0; i<rep.size(); i++)
            mean+=rep[i];
        mean/=rep.size();

        for (int i=0; i<rep.size(); i++)
            stdv+=std::pow(rep[i]-mean, 2);

        stdv = std::sqrt(stdv);
        if (rep.size()>1)
            stdv/=(rep.size()-1);
        else
            stdv = 0.0;

        boost::math::students_t dist(rep.size()-1);
        double t = boost::math::quantile(boost::math::complement(dist, confidence/2.0));
        double delta = t*stdv / std::sqrt(static_cast<double>(rep.size()));

        return Confidence_inter{mean-delta, mean+delta, mean};

    }

};

void read_classes(std::string fname, std::vector<double>& p, std::vector<int>& sizes, std::vector<double>& mus) {
    std::vector<std::vector<std::string>> content;
    std::vector<std::string> row;
    std::string line, word;
    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
        while(getline(file, line))
        {
            row.clear();
     
            std::stringstream str(line);
     
            while(getline(str, word, ','))
                row.push_back(word);
            content.push_back(row);
        }
    }
    else
        std::cout<<"Could not open the file\n";
    

    for(int i=0;i<content.size();i++)
    {
        content[i][0].erase(content[i][0].begin());
        content[i][2].pop_back();
               
        sizes.push_back(std::stoi(content[i][0]));
        p.push_back(std::stod(content[i][1]));
        mus.push_back(std::stod(content[i][2]));

    }
    
    double sum = 0.0;
    for (auto x: p)
        sum += x;
    
    for (auto& x: p)
        x/=sum;

}

void read_lambdas(const std::string& filename, std::vector<double>& values) {
     // Open the file
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open the file." << std::endl;
        return;
    }

    std::string line;
    std::string content;

    // Read the file line by line
    while (std::getline(file, line)) {
        content += line;
    }

    // Close the file
    file.close();

    // Find the opening and closing square brackets
    size_t start = content.find('[');
    size_t end = content.find(']');

    if (start != std::string::npos && end != std::string::npos) {
        // Extract the content within square brackets
        content = content.substr(start + 1, end - start - 1);

        // Parse the content to extract double values
        std::istringstream iss(content);
        std::string token;
        while (std::getline(iss, token, ',')) {
            values.push_back(std::stod(token)); // Convert the token to a double
        }
    } else {
        std::cerr << "No valid double values enclosed in square brackets found in the file." << std::endl;
    }
}


void run_simulation(Experiment e,
                    unsigned long events,
                    int repetitions,
                    std::vector<Confidence_inter>* res_buf,  //out: occupancy buffer per class
                    std::vector<Confidence_inter>* res_ser,  //out: occupancy servers per class
                    std::vector<Confidence_inter>* res_th,   //out: throughput per class
                    std::vector<Confidence_inter>* res_wait, //output: waiting time per class
                    std::vector<Confidence_inter>* res_wait_var, //output: waiting time variance per class
                    std::vector<Confidence_inter>* res_resp, //output: response time per class
                    std::vector<Confidence_inter>* res_resp_var, //output: waiting time variance per class
                    std::vector<bool>* res_warn,            //out: warning for class stability
                    std::vector<Confidence_inter>* res_preemption,
                    Confidence_inter* res_waste, //out: wasted servers
                    Confidence_inter* res_viol, //out: fifo violations counter
                    Confidence_inter* res_util, //out: wasted servers
                    Confidence_inter* res_buf_tot, //out: total queue length
                    Confidence_inter* res_wait_tot, //out: total waiting time
                    Confidence_inter* res_wait_var_tot, //out: total waiting time variance
                    Confidence_inter* res_resp_tot, //out: total response time
                    Confidence_inter* res_resp_var_tot, //out: total response time
                    Confidence_inter* res_timings, //out: avarage run duration
                    Confidence_inter* res_big_seq_len, //out: average length of big service sequence
                    Confidence_inter* res_small_seq_len, //out: average length of big service sequence
                    Confidence_inter* res_big_seq_dur, //out: average length of big service sequence
                    Confidence_inter* res_small_seq_dur, //out: average length of big service sequence
                    Confidence_inter* res_big_seq_amount, //out: amount of big service sequence
                    Confidence_inter* res_small_seq_amount,
                    Confidence_inter* res_phase_two_duration, 
                    Confidence_inter* res_phase_three_duration,
                    Confidence_inter* res_window_size
                    )
{
    Simulator sim(e.l, e.u, e.s, e.w, e.n, e.sm, e.logf);
    sim.reset_simulation();
    sim.reset_statistics();
    
    sim.simulate(events, repetitions);
    sim.produce_statistics(*res_buf, *res_ser, *res_th, *res_wait, *res_wait_var, *res_resp, *res_resp_var, 
                           *res_warn, *res_preemption, *res_waste, *res_viol, *res_util, *res_buf_tot, 
                           *res_wait_tot, *res_wait_var_tot, *res_resp_tot, *res_resp_var_tot, *res_timings, 
                           *res_big_seq_len, *res_small_seq_len,*res_big_seq_dur, *res_small_seq_dur, *res_big_seq_amount, *res_small_seq_amount, 
                           *res_phase_two_duration, *res_phase_three_duration, *res_window_size);
    
}

int main (int argc, char *argv[]){
    
    std::vector<double> p;
    std::vector<int> sizes; 
    std::vector<double> mus;
    std::vector<double> l;
    std::vector<double> arr_rate;
    std::vector<std::string> headers;
    std::vector<double> input_utils;

    std::string cell = std::string(argv[1]);
    int n = std::stoi(argv[2]);
    int w = std::stoi(argv[3]);
    std::unordered_map<std::string,int> sampling_input = 
    {
        {"exp", 0},
        {"par", 1},
        {"det", 2},
        {"uni", 3},
        {"bpar", 4},
        {"fre", 5}
    };
    int sampling_method = sampling_input[argv[4]]; //0->exp, 1->par, 2->det, 3->uni, 4->bpar
    int n_evs = std::stoi(argv[5]);
    int n_runs = std::stoi(argv[6]);

    std::vector<std::string> sampling_name = {"Exponential", "Pareto", "Deterministic", "Uniform", "BoundedPareto", "Frechet"};

    std::cout << "*** Processing - ID: " << argv[1] << " - N: " << std::to_string(n) << " - Policy: " << std::to_string(w) << " - Sampling: " << sampling_name[sampling_method] << " ***" << std::endl;

    headers = {"Arrival Rate"};
    
    std::string classes_filename = "Inputs/" + cell + ".txt";
    read_classes(classes_filename, p, sizes, mus);
    std::string lambdas_filename = "Inputs/arrRate_" + cell + ".txt";
    read_lambdas(lambdas_filename, arr_rate);
    
    std::string out_filename = "Results/main_allPols/overLambdas-nClasses" + std::to_string(sizes.size()) + "-N" + std::to_string(n) + "-Win" + std::to_string(w) + "-" + sampling_name[sampling_method] + "-" + cell + ".csv";
    std::ofstream outputFile(out_filename, std::ios::app);
    std::vector<Experiment> ex;

    

    for (int i=0; i<arr_rate.size(); i++) {
        l.clear();
        for (auto x: p) {
            l.push_back(x*arr_rate[i]);
        }
        std::string logfile_name = "Results/logfile-nClasses" + std::to_string(sizes.size()) + "-N" + std::to_string( n ) + "-Win" + std::to_string( w ) + "-" + sampling_name[sampling_method] + "-" + cell + "-" + "-lambda" + std::to_string(arr_rate[i]) + ".csv";
        ex.push_back(Experiment{l, mus, sizes, w, n, sampling_method, logfile_name});
    }
    
    std::vector<std::vector<Confidence_inter>> res_buf(ex.size());
    std::vector<std::vector<Confidence_inter>> res_ser(ex.size());
    std::vector<std::vector<Confidence_inter>> res_th(ex.size());
    std::vector<std::vector<Confidence_inter>> res_wait(ex.size());
    std::vector<std::vector<Confidence_inter>> res_wait_var(ex.size());
    std::vector<std::vector<Confidence_inter>> res_resp(ex.size());
    std::vector<std::vector<Confidence_inter>> res_resp_var(ex.size());
    std::vector<std::vector<Confidence_inter>> res_preemption(ex.size());

    std::vector<Confidence_inter> res_waste(ex.size());
    std::vector<Confidence_inter> res_viol(ex.size());
    std::vector<Confidence_inter> res_util(ex.size());
    std::vector<Confidence_inter> res_buf_tot(ex.size());
    std::vector<Confidence_inter> res_wait_tot(ex.size());
    std::vector<Confidence_inter> res_wait_var_tot(ex.size());
    std::vector<Confidence_inter> res_resp_tot(ex.size());
    std::vector<Confidence_inter> res_resp_var_tot(ex.size());
    std::vector<Confidence_inter> res_timings(ex.size());
    std::vector<Confidence_inter> res_window_size(ex.size());

    // sequences
    std::vector<Confidence_inter> res_big_avg_len(ex.size());
    std::vector<Confidence_inter> res_small_avg_len(ex.size());
    std::vector<Confidence_inter> res_big_avg_dur(ex.size());
    std::vector<Confidence_inter> res_small_avg_dur(ex.size());
    std::vector<Confidence_inter> res_big_amount(ex.size());
    std::vector<Confidence_inter> res_small_amount(ex.size());

    std::vector<Confidence_inter> res_phase_two_duration(ex.size());
    std::vector<Confidence_inter> res_phase_three_duration(ex.size());

    std::vector<std::vector<bool>> res_warn(ex.size());
    
    std::vector<std::thread> threads(ex.size());

    for (int i = 0; i < ex.size(); i++) {
        threads[i] = std::thread(run_simulation, ex[i], n_evs, n_runs, &res_buf[i], &res_ser[i], &res_th[i], &res_wait[i], &res_wait_var[i],
                    &res_resp[i], &res_resp_var[i], &res_warn[i], &res_preemption[i], &res_waste[i], &res_viol[i], &res_util[i], 
                    &res_buf_tot[i], &res_wait_tot[i], &res_wait_var_tot[i], &res_resp_tot[i], &res_resp_var_tot[i], &res_timings[i], 
                    &res_big_avg_len[i], &res_small_avg_len[i], &res_big_avg_dur[i], &res_small_avg_dur[i], &res_big_amount[i], &res_small_amount[i], 
                    &res_phase_two_duration[i], &res_phase_three_duration[i], &res_window_size[i]);
    }
    for (int i = 0; i < ex.size(); i++) {
        threads[i].join();
    }
    
    for (int ts : sizes) {
        headers.push_back("T" + std::to_string(ts) + " Queue");
        headers.push_back("T" + std::to_string(ts) + " Queue ConfInt");
        headers.push_back("T" + std::to_string(ts) + " Service");
        headers.push_back("T" + std::to_string(ts) + " Service ConfInt");
        headers.push_back("T" + std::to_string(ts) + " System");
        headers.push_back("T" + std::to_string(ts) + " System ConfInt");
        headers.push_back("T" + std::to_string(ts) + " Waiting");
        headers.push_back("T" + std::to_string(ts) + " Waiting ConfInt");
        headers.push_back("T" + std::to_string(ts) + " Waiting Variance");
        headers.push_back("T" + std::to_string(ts) + " Waiting Variance ConfInt");
        headers.push_back("T" + std::to_string(ts) + " Throughput");
        headers.push_back("T" + std::to_string(ts) + " Throughput ConfInt");
        headers.push_back("T" + std::to_string(ts) + " RespTime");
        headers.push_back("T" + std::to_string(ts) + " RespTime ConfInt");
        headers.push_back("T" + std::to_string(ts) + " RespTime Variance");
        headers.push_back("T" + std::to_string(ts) + " RespTime Variance ConfInt");
        headers.push_back("T" + std::to_string(ts) + " Preemption");
        headers.push_back("T" + std::to_string(ts) + " Preemption ConfInt");
        headers.push_back("T" + std::to_string(ts) + " Stability Check");
    }

    headers.insert(headers.end(), {
            "Wasted Servers", "Wasted Servers ConfInt", "Utilisation", "Utilisation ConfInt",
            "Queue Total", "Queue Total ConfInt", "WaitTime Total", "WaitTime Total ConfInt", "WaitTime Variance", "WaitTime Variance ConfInt",
            "RespTime Total", "RespTime Total ConfInt", "RespTime Variance", "RespTime Variance ConfInt",
            "Window Size", "Window Size ConfInt", "FIFO Violations", "FIFO Violations ConfInt", 
            "Run Duration", "Run Duration ConfInt", "Big Sequence Length", "Big Sequence Length ConfInt",
            "Small Sequence Length", "Small Sequence Length ConfInt", "Big Sequence Duration", "Big Sequence Duration ConfInt",
            "Small Sequence Duration", "Small Sequence Duration ConfInt", "Big Sequence Amount", "Big Sequence Amount ConfInt",
            "Small Sequence Amount", "Small Sequence Amount ConfInt", "Phase Two Duration", "Phase Two Duration ConfInt",
            "Phase Three Duration", "Phase Three Duration ConfInt"});

    if (outputFile.tellp() == 0) {
        // Write the headers to the CSV file
        for (const std::string& header : headers) {
            outputFile << header << ";";
        }
        outputFile << "\n";
    }

    for (int i = 0; i < ex.size(); i++) {

        outputFile << arr_rate[i] << ";";
        
        for (int c = 0; c < sizes.size(); c++) {

            outputFile << res_buf.at(i).at(c).mean << ";";
            outputFile << "[" << res_buf.at(i).at(c).min << ", " << res_buf.at(i).at(c).max << "]" << ";";
            outputFile << res_ser.at(i).at(c).mean << ";";
            outputFile << "[" << res_ser.at(i).at(c).min << ", " << res_ser.at(i).at(c).max << "]" << ";";
            outputFile << res_buf.at(i).at(c).mean + res_ser.at(i).at(c).mean << ";";
            outputFile << "[" << res_buf.at(i).at(c).min + res_ser.at(i).at(c).min << ", " << res_buf.at(i).at(c).max + res_ser.at(i).at(c).max << "]" << ";";
            outputFile << res_wait.at(i).at(c).mean << ";";
            outputFile << "[" << res_wait.at(i).at(c).min << ", " << res_wait.at(i).at(c).max << "]" << ";";
            outputFile << res_wait_var.at(i).at(c).mean << ";";
            outputFile << "[" << res_wait_var.at(i).at(c).min << ", " << res_wait_var.at(i).at(c).max << "]" << ";";
            outputFile << res_th.at(i).at(c).mean << ";";
            outputFile << "[" << res_th.at(i).at(c).min << ", " << res_th.at(i).at(c).max << "]" << ";";
            outputFile << res_resp.at(i).at(c).mean << ";";
            outputFile << "[" << res_resp.at(i).at(c).min << ", " << res_resp.at(i).at(c).max << "]" << ";";
            outputFile << res_resp_var.at(i).at(c).mean << ";";
            outputFile << "[" << res_resp_var.at(i).at(c).min << ", " << res_resp_var.at(i).at(c).max << "]" << ";";
            outputFile << res_preemption.at(i).at(c).mean << ";";
            outputFile << "[" << res_preemption.at(i).at(c).min << ", " << res_preemption.at(i).at(c).max << "]" << ";";
            outputFile << res_warn.at(i).at(c) << ";";
        }
        outputFile << res_waste.at(i).mean << ";";
        outputFile << "[" << res_waste.at(i).min << ", " << res_waste.at(i).max << "]" << ";";
        outputFile << res_util.at(i).mean << ";";
        outputFile << "[" << res_util.at(i).min << ", " << res_util.at(i).max << "]" << ";";
        outputFile << res_buf_tot.at(i).mean << ";";
        outputFile << "[" << res_buf_tot.at(i).min << ", " << res_buf_tot.at(i).max << "]" << ";";
        outputFile << res_wait_tot.at(i).mean << ";";
        outputFile << "[" << res_wait_tot.at(i).min << ", " << res_wait_tot.at(i).max << "]"<< ";";
        outputFile << res_wait_var_tot.at(i).mean << ";";
        outputFile << "[" << res_wait_var_tot.at(i).min << ", " << res_wait_var_tot.at(i).max << "]"<< ";";
        outputFile << res_resp_tot.at(i).mean << ";";
        outputFile << "[" << res_resp_tot.at(i).min << ", " << res_resp_tot.at(i).max << "]"<< ";";
        outputFile << res_resp_var_tot.at(i).mean << ";";
        outputFile << "[" << res_resp_var_tot.at(i).min << ", " << res_resp_var_tot.at(i).max << "]"<< ";";
        outputFile << res_window_size.at(i).mean << ";";
        outputFile << "[" << res_window_size.at(i).min << ", " << res_window_size.at(i).max << "]"<< ";";
        outputFile << res_viol.at(i).mean << ";";
        outputFile << "[" << res_viol.at(i).min << ", " << res_viol.at(i).max << "]" << ";";
        outputFile << res_timings.at(i).mean << ";";
        outputFile << "[" << res_timings.at(i).min << ", " << res_timings.at(i).max << "]" << ";";
        outputFile << res_big_avg_len.at(i).mean << ";";
        outputFile << "[" << res_big_avg_len.at(i).min << ", " << res_big_avg_len.at(i).max << "]" << ";";
        outputFile << res_small_avg_len.at(i).mean << ";";
        outputFile << "[" << res_small_avg_len.at(i).min << ", " << res_small_avg_len.at(i).max << "]" << ";";
        outputFile << res_big_avg_dur.at(i).mean << ";";
        outputFile << "[" << res_big_avg_dur.at(i).min << ", " << res_big_avg_dur.at(i).max << "]" << ";";
        outputFile << res_small_avg_dur.at(i).mean << ";";
        outputFile << "[" << res_small_avg_dur.at(i).min << ", " << res_small_avg_dur.at(i).max << "]" << ";";
        outputFile << res_big_amount.at(i).mean << ";";
        outputFile << "[" << res_big_amount.at(i).min << ", " << res_big_amount.at(i).max << "]" << ";";
        outputFile << res_small_amount.at(i).mean << ";";
        outputFile << "[" << res_small_amount.at(i).min << ", " << res_small_amount.at(i).max << "]" << ";";
        outputFile << res_phase_two_duration.at(i).mean << ";";
        outputFile << "[" << res_phase_two_duration.at(i).min << ", " << res_phase_two_duration.at(i).max << "]" << ";";
        outputFile << res_phase_three_duration.at(i).mean << ";";
        outputFile << "[" << res_phase_three_duration.at(i).min << ", " << res_phase_three_duration.at(i).max << "]" << ";";
        outputFile << "\n";
    }

    // Close the file
    outputFile.close();


return 0;
}
