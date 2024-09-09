//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <iostream>
#include <fstream>
#include <iterator>
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
};

class Policy{
public:
    virtual void arrival(int c, int size, int id) = 0;
    virtual void departure(int c, int size, int id) = 0;
    virtual const std::vector<int>& get_state_ser() = 0;
    virtual const std::vector<int>& get_state_buf() = 0;
    virtual const std::vector<std::list<int>>& get_stopped_jobs() = 0;
    virtual const std::vector<std::list<int>>& get_ongoing_jobs() = 0;
    virtual int get_free_ser() = 0;
    virtual int get_violations_counter() = 0;
    virtual void flush_buffer() = 0;
    virtual ~Policy() = 0;
};

Policy::~Policy(){};

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
    
    void arrival(int c, int size, int id) {
        std::tuple<int,int,int> e(c,size,id);
        this->buffer.push_back(e);
        state_buf[c]++;
        flush_buffer();
    }
    
    void departure(int c, int size, int id) {
        std::tuple<int,int,int> e(c,size,id);
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

    const std::vector<std::list<int>>& get_stopped_jobs() {
        return stopped_jobs;
    }

    const std::vector<std::list<int>>& get_ongoing_jobs() {
        return ongoing_jobs;
    }
    
    int get_free_ser() {
        return freeservers;
    }

    int get_violations_counter() {
        return violations_counter;
    }

    ~Smash(){

    }

    
private:
    std::list<std::tuple<int,int,int>> buffer;
    int servers;
    int w;
    std::vector<int> state_buf;
    std::vector<int> state_ser;
    std::vector<std::list<int>> stopped_jobs; //vector of list of ids
    std::vector<std::list<int>> ongoing_jobs; //vector of list of ids
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
              int sampling_method): ru(0.0, 1.0) {
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
        this->debugMode = false;

        this->policy = new  Smash(w, servers, static_cast<int>(sizes.size()));

        occupancy_buf.resize(sizes.size());
        occupancy_ser.resize(sizes.size());
        completion.resize(sizes.size());
        throughput.resize(sizes.size());
        waitingTime.resize(sizes.size());
        responseTime.resize(sizes.size());
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
        for (auto& e: completion)
            e=0;
        for (auto& e: rep_free_servers_distro)
            e=0;
        for (auto& e: fel)
            e-=simtime;
        
        if (this->w == -2 || this->sampling_method != 0) {
            for (int i = 0; i < jobs_inservice.size(); ++i) {
                for (auto job = jobs_inservice[i].begin(); job != jobs_inservice[i].end(); ++job) {
                    jobs_inservice[i][job->first] -= simtime;
                }
            }
        }
        last_busy_period_start -= simtime;
        last_idle_period_start -= simtime;
        last_busy_period_start_big -= simtime;
        last_busy_period_start_small -= simtime;
        
        simtime = 0.0;
        util = 0.0;
        waste = 0.0;
        viol = 0.0;
        occ = 0.0;
    }

      
    void simulate(unsigned long nevents, int repetitions=1) {
        rep_th.clear();
        rep_occupancy_buf.clear();
        rep_occupancy_ser.clear();
        rep_wait.clear();
        rep_resp.clear();
        rep_waste.clear();
        rep_viol.clear();
        rep_util.clear();
        rep_tot_buf.clear();
        rep_tot_wait.clear();
        rep_tot_resp.clear();
        rep_timings.clear();
        rep_big_sequences.clear();
        rep_big_seq_amount.clear();
        rep_big_seq_max_len.clear();
        rep_small_sequences.clear();
        rep_small_seq_amount.clear();
        rep_small_seq_max_len.clear();
        
        rep_n_small.clear();
        rep_n_big.clear();

        rep_idle_periods_counter.clear();
        rep_prob_idle_period.clear();
       
        for (int rep=0; rep<repetitions; rep++) {
        	double current_big_seq_len = 0.0;
            double tot_big_seq_len = 0.0;
            double big_seq_amount = 0.0;
            double max_big_seq_len = 0.0;
            double current_small_seq_len = 0.0;
            double tot_small_seq_len = 0.0;
            double small_seq_amount = 0.0;
            double max_small_seq_len = 0.0;

            double n_small = 0.0;
            double n_big = 0.0;
            double n_amount_small = 0.0;
            double n_amount_big = 0.0;

            double idle_periods_counter = 0.0;
            double busy_periods_counter = 0.0;
            auto tot_idle_time = 0.0;
            auto tot_busy_time = 0.0;

        	auto start = std::chrono::high_resolution_clock::now();
            reset_statistics();

            for (unsigned long int k=0; k<nevents; k++) {
                int big_state;
                int small_state;
                int counting_small;
                auto itmin = std::min_element(fel.begin(), fel.end());
                int pos = static_cast<int>(itmin-fel.begin());
                collect_statistics(pos);
                
                if (pos<nclasses) { //departure
                    if (this->w == -2 || this->sampling_method != 0) {
                        jobs_inservice[pos].erase(job_fel[pos]); // Remove jobs from in_service (they cannot be in preempted list)
                    }

                    if (pos == nclasses-1) {
                        current_big_seq_len += 1;

                        policy->departure(pos,sizes[pos],job_fel[pos]);

                        auto ser_state = policy->get_state_ser();
                        if (ser_state[1] == 0){
                            big_seq_amount += 1; 
                            if (max_big_seq_len < current_big_seq_len) { 
                                max_big_seq_len = current_big_seq_len;
                            }
                            tot_big_seq_len += current_big_seq_len;
                            current_big_seq_len = 0;
                            busy_periods_len_big.push_back(*itmin - last_busy_period_start_big);

                            if (ser_state[0] > 0){ 
                                n_small += policy->get_state_buf()[0] + policy->get_state_ser()[0];
                                n_amount_small += 1;
                                last_busy_period_start_small = *itmin;
                            }else{ 
                                idle_period = 1;
                                last_idle_period_start = *itmin;
                                tot_busy_time += *itmin - last_busy_period_start;
                                busy_periods_len.push_back(*itmin - last_busy_period_start);
                                busy_periods_counter += 1;
                            }
                        }
                    } else {
                        current_small_seq_len += 1;

                        policy->departure(pos,sizes[pos],job_fel[pos]);

                        auto ser_state = policy->get_state_ser();

                        if (ser_state[0] == 0){
                            small_seq_amount += 1; 
                            if (max_small_seq_len < current_small_seq_len) { 
                                max_small_seq_len = current_small_seq_len;
                            }
                            tot_small_seq_len += current_small_seq_len;
                            current_small_seq_len = 0;
                            busy_periods_len_small.push_back(*itmin - last_busy_period_start_small);

                            if (ser_state[1] > 0){ 
                                n_big += policy->get_state_buf()[1] + policy->get_state_ser()[1];
                                n_amount_big += 1;
                                last_busy_period_start_big = *itmin;
                            }else{ 
                                idle_period = 1;
                                last_idle_period_start = *itmin;
                                tot_busy_time += *itmin - last_busy_period_start;
                                busy_periods_len.push_back(*itmin - last_busy_period_start);
                                busy_periods_counter += 1;
                            }
                        }
                    }
                }
                else {
                    if (idle_period == 1) {
                        idle_period = 0;
                        tot_idle_time += (*itmin - last_idle_period_start);
                        idle_periods_len.push_back(*itmin - last_idle_period_start);
                        idle_periods_counter += 1;
                        last_busy_period_start = *itmin;

                        if (pos-nclasses == 0) {
                            n_small += 1;
                            n_amount_small += 1;
                            last_busy_period_start_small = *itmin;
                        } else {
                            n_big += 1;
                            n_amount_big += 1;
                            last_busy_period_start_big = *itmin;
                        }
                    }
                    policy->arrival(pos-nclasses,sizes[pos-nclasses],(k+(nevents*rep)));
                }
                
                simtime = *itmin;
                
                resample();
            }

            double avg_big_seq_len = (tot_big_seq_len*1.0)/big_seq_amount;
            double avg_small_seq_len = (tot_small_seq_len*1.0)/small_seq_amount;

            double avg_n_big = (n_big*1.0)/n_amount_big;
            double avg_n_small = (n_small*1.0)/n_amount_small;

            
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
            for (int i=0; i< occupancy_buf.size(); i++) {
                waitingTime[i] = occupancy_buf[i] / throughput[i];
                responseTime[i] = (occupancy_buf[i]+occupancy_ser[i]) / throughput[i];
                totx += throughput[i];
            }

            for (int i=0; i<occupancy_ser.size(); i++)
                util += occupancy_ser[i]*sizes[i];

            rep_wait.push_back(waitingTime);
            rep_resp.push_back(responseTime);
            rep_waste.push_back(waste/simtime);
            rep_viol.push_back(viol/simtime);
            rep_util.push_back(util/n);
            rep_tot_buf.push_back(totq);
            rep_tot_wait.push_back(totq/totx);
            rep_tot_resp.push_back((totq+tots)/totx);
            rep_big_sequences.push_back(avg_big_seq_len);
            rep_big_seq_amount.push_back(big_seq_amount);
            rep_big_seq_max_len.push_back(max_big_seq_len);
            rep_small_sequences.push_back(avg_small_seq_len);
            rep_small_seq_amount.push_back(small_seq_amount);
            rep_small_seq_max_len.push_back(max_small_seq_len);
            rep_n_big.push_back(avg_n_big);
            rep_n_small.push_back(avg_n_small);
            rep_idle_periods_counter.push_back(idle_periods_counter);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

            rep_prob_idle_period.push_back(tot_idle_time/simtime);
            rep_timings.push_back(duration);

            
            std::cout << "Repetition " << std::to_string(rep) << " Done" << std::endl;
            
        }
        for (auto& x: rep_free_servers_distro) {
            x /= simtime;
        }
    }

    void produce_statistics(std::vector<Confidence_inter>& occ_buff, std::vector<Confidence_inter>& occ_ser, std::vector<Confidence_inter>& throughput, std::vector<Confidence_inter>& waitTime, std::vector<Confidence_inter>& respTime,  std::vector<bool>& warnings, Confidence_inter &wasted, Confidence_inter &violations, Confidence_inter &utilisation, Confidence_inter &occ_tot, Confidence_inter &wait_tot, Confidence_inter &resp_tot, Confidence_inter &timings_tot, Confidence_inter &big_sequence, Confidence_inter &big_seq_amount, Confidence_inter &max_big_seq_len, Confidence_inter &small_sequence, Confidence_inter &small_seq_amount, Confidence_inter &max_small_seq_len, Confidence_inter &nstar_small, Confidence_inter &nstar_big, Confidence_inter &idle_counter, Confidence_inter &idle_prob, double confidence = 0.05) {
        for (int i=0; i<nclasses; i++) {
            occ_buff.push_back(compute_class_interval(rep_occupancy_buf, i, confidence));
            occ_ser.push_back(compute_class_interval(rep_occupancy_ser, i, confidence));
            throughput.push_back(compute_class_interval(rep_th, i, confidence));
            waitTime.push_back(compute_class_interval(rep_wait, i, confidence));
            respTime.push_back(compute_class_interval(rep_resp, i, confidence));

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
        resp_tot = compute_interval(rep_tot_resp, confidence);
        timings_tot = compute_interval(rep_timings, confidence);
        big_sequence = compute_interval(rep_big_sequences, confidence);
        big_seq_amount = compute_interval(rep_big_seq_amount, confidence);
        max_big_seq_len = compute_interval(rep_big_seq_max_len, confidence);
        small_sequence = compute_interval(rep_small_sequences, confidence);
        small_seq_amount = compute_interval(rep_small_seq_amount, confidence);
        max_small_seq_len = compute_interval(rep_small_seq_max_len, confidence);
        nstar_big = compute_interval(rep_n_big, confidence);
        nstar_small = compute_interval(rep_n_small, confidence);
        idle_counter = compute_interval(rep_idle_periods_counter, confidence);
        idle_prob = compute_interval(rep_prob_idle_period, confidence);

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
    std::vector<std::vector<double>> rep_resp;
    std::vector<double> rep_tot_wait;
    std::vector<double> rep_tot_resp;
    std::vector<double> rep_timings;
    std::vector<double> rep_tot_buf;
    std::vector<double> rep_waste;
    std::vector<double> rep_viol;
    std::vector<double> rep_util;
    std::vector<double> rep_big_sequences;
    std::vector<double> rep_big_seq_amount;
    std::vector<double> rep_big_seq_max_len;
    std::vector<double> rep_small_sequences;
    std::vector<double> rep_small_seq_amount;
    std::vector<double> rep_small_seq_max_len;
    std::vector<double> rep_n_big;
    std::vector<double> rep_n_small;
    std::vector<double> rep_idle_periods_counter;
    std::vector<double> rep_prob_idle_period;

    //statistics for single run
    std::vector<double> occupancy_buf;
    std::vector<double> occupancy_ser;
    std::vector<unsigned long> completion;
    std::vector<double> throughput;
    std::vector<double> waitingTime;
    std::vector<double> responseTime;

    std::vector<double> idle_periods_len;
    std::vector<double> busy_periods_len;
    std::vector<double> busy_periods_len_small;
    std::vector<double> busy_periods_len_big;

    double lambda;
    double waste = 0.0;
    double viol = 0.0;
    double util = 0.0;
    double occ = 0.0;

    double last_idle_period_start = 0.0;
    double last_busy_period_start = 0.0;
    double last_busy_period_start_small = 0.0;
    double last_busy_period_start_big = 0.0;
    int idle_period = 1;
    
    std::vector<double> fel;
    std::vector<int> job_fel;
    //std::vector<std::list<double>> fels;
    //std::list<int> job_ids;
    std::vector<std::unordered_map<int,double>> jobs_inservice; //[id, time_end]
    std::vector<std::unordered_map<int,double>> jobs_preempted; //[id, time_left]
    
    std::mt19937_64* generator;
    std::uniform_real_distribution<double> ru;

    int sampling_method;
    double alfa = 2;
    double mean_ratio = (alfa-1)/alfa;
    
    double sample_exp(double par) {
        return -log(ru(*generator))/par;
    }

    double sample_pareto(double xm) {
        return xm*exp(sample_exp(alfa)); //xm = 0.5*mean
    }

    double sample_bounded_pareto(double min, double max) {
        return pow((alfa*pow(min,alfa))/((ru(*generator))*(1-(pow(min/max,alfa)))),1/(alfa+1));
    }

    double sample_bPareto(double par) {
        double l = (120000.0/239999.0)*par;
        double h =  120000*par;
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
        } else {
            std::cout << "ERROR - undefined behaviour: not valid sampling_method" << std::endl;
            return 0.0;
        }
    }
    
    void resample() {
        //add arrivals and departures
        if (this->sampling_method != 0) { //exponential distro can use the faster memoryless blocks
            auto ongoing_jobs = policy->get_ongoing_jobs();
            for (int i=0; i<nclasses; i++) {
                if (fel[i+nclasses] <= simtime) { // only update arrival that is executed at the time
                    fel[i+nclasses] = sample_exp(l[i]) + simtime;
                }

                for (int job_id : ongoing_jobs[i]) {
                    jobs_inservice[i][job_id] = sample_st(1/u[i]) + simtime;
                    if (jobs_inservice[i][job_id] < fel[i]) {
                        fel[i] = jobs_inservice[i][job_id];
                        job_fel[i] = job_id;
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

        if (debugMode) {
            std::cout << "************" << std::endl;
            this->printJobs();
            this->printFel();
            std::cout << "-----------------------" << std::endl;
        }

    }

    void printFel() {
        std::cout << "Fel (class-ids-time): " << std::endl;
        for (int i = 0; i < fel.size(); i++) {
            std::cout << i << "-" << job_fel[i] << "-" << fel[i] << ", ";
        }
        std::cout << std::endl;
    }

    void printJobs() {
        std::cout << "Jobs in service (class-ids-time): " << std::endl;
        for (int i = 0; i < jobs_inservice.size(); i++) {
            for (auto& e : jobs_inservice[i]) {
                std::cout << i << "-" << std::get<0>(e) << "-" << std::get<1>(e) << ", ";
            }
        }
        std::cout << std::endl;
        std::cout << "Jobs preempted (class-ids-time): " << std::endl;
        for (int i = 0; i < jobs_preempted.size(); i++) {
            for (auto& e : jobs_preempted[i]) {
                std::cout << i << "-" << std::get<0>(e) << "-" << std::get<1>(e) << ", ";
            }
        }
        std::cout << std::endl;
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
                    std::vector<Confidence_inter>* res_resp, //output: response time per class
                    std::vector<bool>* res_warn,            //out: warning for class stability
                    Confidence_inter* res_waste, //out: wasted servers
                    Confidence_inter* res_viol, //out: fifo violations counter
                    Confidence_inter* res_util, //out: wasted servers
                    Confidence_inter* res_buf_tot, //out: total queue length
                    Confidence_inter* res_wait_tot, //out: total waiting time
                    Confidence_inter* res_resp_tot, //out: total response time
                    Confidence_inter* res_timings, //out: avarage run duration
                    Confidence_inter* res_big_seq, //out: average length of big service sequence
                    Confidence_inter* res_amount_big_seq, //out: amount of big service sequence
                    Confidence_inter* res_big_seq_max_len, //out: maximum length of big service sequence
                    Confidence_inter* res_small_seq, //out: average length of small service sequence
                    Confidence_inter* res_amount_small_seq, //out: amount of small service sequence
                    Confidence_inter* res_small_seq_max_len, //out: maximum length of small service sequence
                    Confidence_inter* res_nstar_small, // out: avg. number of small jobs in the queue at the beginning of a small class busy period
                    Confidence_inter* res_nstar_big, // out: avg. number of big jobs in the queue at the beginning of a big class busy period
                    Confidence_inter* res_idle_counter, // out: avg. number of idle periods 
                    Confidence_inter* res_idle_prob // out: avg. probability of an idle period
                    )
{
    Simulator sim(e.l, e.u, e.s, e.w, e.n, e.sm);
    sim.reset_simulation();
    sim.reset_statistics();
    
    sim.simulate(events, repetitions);
    sim.produce_statistics(*res_buf, *res_ser, *res_th, *res_wait, *res_resp, *res_warn, *res_waste, *res_viol, *res_util, *res_buf_tot, *res_wait_tot, *res_resp_tot, *res_timings, *res_big_seq, *res_amount_big_seq, *res_big_seq_max_len, *res_small_seq, *res_amount_small_seq, *res_small_seq_max_len, *res_nstar_small, *res_nstar_big, *res_idle_counter, *res_idle_prob);
    
}


int main (int argc, char *argv[]){

    for (int w=1; w<=6; w++) {
    
        std::vector<double> p;
        std::vector<int> sizes; 
        std::vector<double> mus;
        std::vector<double> l;
        std::vector<double> arr_rate;
        std::vector<std::string> headers;
        std::vector<double> input_utils;

        int n = std::stoi(argv[1]);
        //int w = std::stoi(argv[3]);
        std::unordered_map<std::string,int> sampling_input = 
        {
            {"exp", 0},
            {"par", 1},
            {"det", 2},
            {"uni", 3},
            {"bpar", 4},
            {"fre", 5}
        };
        int sampling_method = sampling_input[argv[2]]; //0->exp, 1->par, 2->det, 3->uni, 4->bpar
        //std::string type = std::string(argv[5])+"_"+std::to_string(n);
        std::string type = std::string(argv[3]);
        int n_evs = std::stoi(argv[4]);
        int n_runs = std::stoi(argv[5]);

        std::vector<std::string> sampling_name = {"Exponential", "Pareto", "Deterministic", "Uniform", "BoundedPareto", "Frechet"};

        std::cout << "*** Processing - N: " << std::to_string(n) << " - Policy: " << std::to_string(w) << " - Type: " << type << " - Sampling: " << sampling_name[sampling_method] << " ***" << std::endl;

        headers = {"Arrival Rate"};


        /*std::string classes_filename = "Inputs/" + cell + "/" + cell + "_" + type + ".txt";
        read_classes(classes_filename, p, sizes, mus);
        std::string lambdas_filename ="Inputs/" + cell + "/arrRate_" + type + ".txt"; //"Inputs/arr_combined.txt"; "Inputs/smaller_arrRate.txt";
        read_lambdas(lambdas_filename, arr_rate);*/

        std::string classes_filename = "Inputs/" + type + ".txt";
        read_classes(classes_filename, p, sizes, mus);
        std::string lambdas_filename ="Inputs/arrRate_" + type + "_W" + std::to_string(w) + ".txt"; 
        read_lambdas(lambdas_filename, arr_rate);
        
        std::string out_filename = "simulation_results_N" + std::to_string(n) + "_T" + std::to_string(sizes[1]) + "_W" + std::to_string(w)+ "_" + sampling_name[sampling_method] + "_ps" + std::to_string(p[0]) + "_mus" + std::to_string(1/mus[0]) + "_mub" + std::to_string(1/mus[1]) + ".csv";
        remove(out_filename.c_str());
        std::ofstream outputFile(out_filename, std::ios::app);
        std::vector<Experiment> ex;
        
        for (int i=0; i<arr_rate.size(); i++) {
            l.clear();
            for (auto x: p) {
                l.push_back(x*arr_rate[i]);
            }
            l.push_back(arr_rate[i]);
            ex.push_back(Experiment{l, mus, sizes, w, n, sampling_method});

            /*double rho = 0.0;
                for (int j=0; j<l.size(); j++) {
                    rho += l[j]*ex.at(ex.size()-1).s.at(j) *ex.at(ex.size()-1).u.at(j);
                }
                rho/=n;
                input_utils.push_back(rho);*/
        }
        
        std::vector<std::vector<Confidence_inter>> res_buf(ex.size());
        std::vector<std::vector<Confidence_inter>> res_ser(ex.size());
        std::vector<std::vector<Confidence_inter>> res_th(ex.size());
        std::vector<std::vector<Confidence_inter>> res_wait(ex.size());
        std::vector<std::vector<Confidence_inter>> res_resp(ex.size());

        std::vector<Confidence_inter> res_waste(ex.size());
        std::vector<Confidence_inter> res_viol(ex.size());
        std::vector<Confidence_inter> res_util(ex.size());
        std::vector<Confidence_inter> res_buf_tot(ex.size());
        std::vector<Confidence_inter> res_wait_tot(ex.size());
        std::vector<Confidence_inter> res_resp_tot(ex.size());
        std::vector<Confidence_inter> res_timings(ex.size());
        std::vector<Confidence_inter> res_big_seq(ex.size());
        std::vector<Confidence_inter> res_amount_big_seq(ex.size());
        std::vector<Confidence_inter> res_big_seq_max_len(ex.size());
        std::vector<Confidence_inter> res_small_seq(ex.size());
        std::vector<Confidence_inter> res_amount_small_seq(ex.size());
        std::vector<Confidence_inter> res_small_seq_max_len(ex.size());
        std::vector<Confidence_inter> res_nstar_big(ex.size());
        std::vector<Confidence_inter> res_nstar_small(ex.size());
        std::vector<Confidence_inter> res_idle_counter(ex.size());
        std::vector<Confidence_inter> res_idle_prob(ex.size());

        std::vector<std::vector<bool>> res_warn(ex.size());
        
        std::vector<std::thread> threads(ex.size());

        for (int i = 0; i < ex.size(); i++) {
            threads[i] = std::thread(run_simulation, ex[i], n_evs, n_runs, &res_buf[i], &res_ser[i], &res_th[i], &res_wait[i], &res_resp[i], &res_warn[i], &res_waste[i], &res_viol[i], &res_util[i], &res_buf_tot[i], &res_wait_tot[i], &res_resp_tot[i], &res_timings[i], &res_big_seq[i], &res_amount_big_seq[i], &res_big_seq_max_len[i], &res_small_seq[i], &res_amount_small_seq[i], &res_small_seq_max_len[i], &res_nstar_small[i], &res_nstar_big[i], &res_idle_counter[i], &res_idle_prob[i]);
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
            headers.push_back("T" + std::to_string(ts) + " Throughput");
            headers.push_back("T" + std::to_string(ts) + " Throughput ConfInt");
            headers.push_back("T" + std::to_string(ts) + " RespTime");
            headers.push_back("T" + std::to_string(ts) + " RespTime ConfInt");
            headers.push_back("T" + std::to_string(ts) + " Stability Check");
        }

        headers.insert(headers.end(), {
                "Wasted Servers", "Wasted Servers ConfInt", "Utilisation", "Utilisation ConfInt",
                "Queue Total", "Queue Total ConfInt", "WaitTime Total", "WaitTime Total ConfInt",
                "RespTime Total", "RespTime Total ConfInt", "FIFO Violations", "FIFO Violations ConfInt", 
                "Exe. Time", "Exe. Time ConfInt", "Big Sequence Lenght", "Big Sequence Lenght ConfInt",
                "Big Sequence Amount", "Big Sequence Amount ConfInt", "Max Big Sequence Length",  "Max Big Sequence Length ConfInt",
                "Small Sequence Lenght", "Small Sequence Lenght ConfInt", "Small Sequence Amount", 
                "Small Sequence Amount ConfInt", "Max Small Sequence Length",  "Max Small Sequence Length ConfInt",
                "Nstar Big", "Nstar Big ConfInt", "Nstar Small", "Nstar Small ConfInt", "Idle Counter", "Idle Counter ConfInt",
                "Idle Prob", "Idle Prob ConfInt"});

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
                outputFile << res_th.at(i).at(c).mean << ";";
                outputFile << "[" << res_th.at(i).at(c).min << ", " << res_th.at(i).at(c).max << "]" << ";";
                outputFile << res_resp.at(i).at(c).mean << ";";
                outputFile << "[" << res_resp.at(i).at(c).min << ", " << res_resp.at(i).at(c).max << "]" << ";";
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
            outputFile << res_resp_tot.at(i).mean << ";";
            outputFile << "[" << res_resp_tot.at(i).min << ", " << res_resp_tot.at(i).max << "]"<< ";";
            outputFile << res_viol.at(i).mean << ";";
            outputFile << "[" << res_viol.at(i).min << ", " << res_viol.at(i).max << "]" << ";";
            outputFile << res_timings.at(i).mean << ";";
            outputFile << "[" << res_timings.at(i).min << ", " << res_timings.at(i).max << "]" << ";";
            outputFile << res_big_seq.at(i).mean << ";";
            outputFile << "[" << res_big_seq.at(i).min << ", " << res_big_seq.at(i).max << "]" << ";";
            outputFile << res_amount_big_seq.at(i).mean << ";";
            outputFile << "[" << res_amount_big_seq.at(i).min << ", " << res_amount_big_seq.at(i).max << "]" << ";";
            outputFile << res_big_seq_max_len.at(i).mean << ";";
            outputFile << "[" << res_big_seq_max_len.at(i).min << ", " << res_big_seq_max_len.at(i).max << "]"<< ";";
            outputFile << res_small_seq.at(i).mean << ";";
            outputFile << "[" << res_small_seq.at(i).min << ", " << res_small_seq.at(i).max << "]" << ";";   
            outputFile << res_amount_small_seq.at(i).mean << ";";
            outputFile << "[" << res_amount_small_seq.at(i).min << ", " << res_amount_small_seq.at(i).max << "]" << ";";
            outputFile << res_small_seq_max_len.at(i).mean << ";";
            outputFile << "[" << res_small_seq_max_len.at(i).min << ", " << res_small_seq_max_len.at(i).max << "]" << ";";
            outputFile << res_nstar_big.at(i).mean << ";";
            outputFile << "[" << res_nstar_big.at(i).min << ", " << res_nstar_big.at(i).max << "]" << ";";
            outputFile << res_nstar_small.at(i).mean << ";";
            outputFile << "[" << res_nstar_small.at(i).min << ", " << res_nstar_small.at(i).max << "]" << ";";
            outputFile << res_idle_counter.at(i).mean << ";";
            outputFile << "[" << res_idle_counter.at(i).min << ", " << res_idle_counter.at(i).max << "]" << ";";
            outputFile << res_idle_prob.at(i).mean << ";";
            outputFile << "[" << res_idle_prob.at(i).min << ", " << res_idle_prob.at(i).max << "]" << ";";
            outputFile << "\n";
        }

        // Close the file
        outputFile.close();
    }


return 0;
}
