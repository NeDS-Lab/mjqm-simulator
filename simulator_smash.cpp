//  main.cpp
//  Simula_smash
//
//  Created by Andrea Marin on 13/10/23.
//

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <pthread.h>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include "math/confidence_intervals.h"
#include "math/samplers.h"
#include "policies/policies.h"
#include "settings/experiment.hpp"
#include "settings/loader.hpp"
#include "stats/stats.h"


class Simulator
{
public:
    Simulator(const std::vector<double>& l, const std::vector<double>& u, const std::vector<int>& sizes, int w,
              int servers, int sampling_method, std::string logfile_name)
    {
        this->l = l;
        this->u = u;
        this->n = servers;
        this->sizes = sizes;
        this->w = w;
        this->sampling_method = sampling_method;
        this->rep_free_servers_distro = std::vector<double>(servers + 1);
        this->nclasses = static_cast<int>(sizes.size());
        this->fel.resize(sizes.size() * 2);
        this->job_fel.resize(sizes.size() * 2);
        this->jobs_inservice.resize(sizes.size());
        this->jobs_preempted.resize(sizes.size());
        this->curr_job_seq.resize(sizes.size());
        this->tot_job_seq.resize(sizes.size());
        this->curr_job_seq_start.resize(sizes.size());
        this->tot_job_seq_dur.resize(sizes.size());
        this->job_seq_amount.resize(sizes.size());
        this->debugMode = false;
        this->logfile_name = std::move(logfile_name);

        if (w == 0)
        {
            this->policy = new MostServerFirst(w, servers, nclasses, sizes);
        }
        else if (w == -1)
        {
            this->policy = new ServerFilling(w, servers, nclasses);
        }
        else if (w == -2)
        {
            this->policy = new ServerFillingMem(w, servers, nclasses);
        }
        else if (w == -3)
        {
            this->policy = new BackFilling(w, servers, nclasses, sizes);
        }
        else if (w == -4)
        {
            this->policy = new MostServerFirstSkip(w, servers, nclasses, sizes);
        }
        else if (w == -5)
        {
            this->policy =
                new MostServerFirstSkipThreshold(w, servers, nclasses, sizes, l[0], 1 / u[0]);
        }
        else
        {
            this->policy = new Smash(w, servers, nclasses);
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
        generator = std::make_shared<std::mt19937_64>(next(seed));

        switch (sampling_method)
        {
        case 0: // exponential
            for (int i = 0; i < nclasses; i++)
            {
                this->class_samplers.push_back(exponential::with_mean(generator, u[i]));
            }
            break;
        case 1: // pareto
            for (int i = 0; i < nclasses; i++)
            {
                this->class_samplers.push_back(pareto::with_mean(generator, u[i], 2));
            }
            break;
        case 2: // deterministic
            for (int i = 0; i < nclasses; i++)
            {
                this->class_samplers.push_back(deterministic::with_mean(u[i]));
            }
            break;
        case 4: // bounded pareto
            for (int i = 0; i < nclasses; i++)
            {
                this->class_samplers.push_back(bounded_pareto::with_mean(generator, u[i], 2));
            }
            break;
        case 5: // frechet
            for (int i = 0; i < nclasses; i++)
            {
                this->class_samplers.push_back(frechet::with_mean(generator, u[i], 2.15));
            }
            break;
        case 3: // uniform
        default:
            for (int i = 0; i < nclasses; i++)
            {
                this->class_samplers.push_back(uniform::with_mean(generator, u[i]));
            }
            break;
        }
    }

    ~Simulator() { delete policy; }

    void reset_simulation()
    {
        simtime = 0.0;
        resample();
    }

    std::uint64_t next(std::uint64_t u)
    {
        std::uint64_t v = u * 3935559000370003845 + 2691343689449507681;

        v ^= v >> 21;
        v ^= v << 37;
        v ^= v >> 4;

        v *= 4768777513237032717;

        v ^= v << 20;
        v ^= v >> 41;
        v ^= v << 5;

        return v;
    }

    void reset_statistics()
    {
        for (auto& e : occupancy_ser)
            e = 0;
        for (auto& e : occupancy_buf)
            e = 0;
        for (auto& e : rawWaitingTime)
            e.clear();
        for (auto& e : rawResponseTime)
            e.clear();
        for (auto& e : completion)
            e = 0;
        for (auto& e : curr_job_seq)
            e = 0;
        for (auto& e : tot_job_seq)
            e = 0;
        for (auto& e : curr_job_seq_start)
            e -= simtime;
        for (auto& e : tot_job_seq_dur)
            e = 0;
        for (auto& e : job_seq_amount)
            e = 0;
        for (auto& e : preemption)
            e = 0;
        for (auto& e : rep_free_servers_distro)
            e = 0;
        for (auto& e : fel)
            e -= simtime;

        if (this->w == -2 || this->sampling_method != 10)
        {
            for (int i = 0; i < jobs_inservice.size(); ++i)
            {
                for (auto job = jobs_inservice[i].begin(); job != jobs_inservice[i].end(); ++job)
                {
                    jobs_inservice[i][job->first] -= simtime;
                }
            }

            for (auto job_id = arrTime.begin(); job_id != arrTime.end(); ++job_id)
            {
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

    void collect_run_statistics(double tot_seq_len, double seq_amount)
    {
        double avg_seq_len = (tot_seq_len * 1.0) / seq_amount;

        double totq = 0.0;
        for (auto& x : occupancy_buf)
        {
            x /= simtime;
            totq += x;
        }


        rep_occupancy_buf.push_back(occupancy_buf);

        double tots = 0.0;
        for (auto& x : occupancy_ser)
        {
            x /= simtime;
            tots += x;
        }
        rep_occupancy_ser.push_back(occupancy_ser);

        for (int i = 0; i < nclasses; i++)
            throughput[i] = completion[i] / simtime;
        rep_th.push_back(throughput);

        double totx = 0.0;
        std::list<double> totRawWaitingTime;
        std::list<double> totRawResponseTime;
        std::vector<double> preemption_avg;
        preemption_avg.resize(nclasses);
        for (int i = 0; i < nclasses; i++)
        {
            // waitingTime[i] = occupancy_buf[i] / throughput[i];
            double mean_wt =
                std::accumulate(rawWaitingTime[i].begin(), rawWaitingTime[i].end(), 0.0) / rawWaitingTime[i].size();
            waitingTime[i] = mean_wt;

            double tot_diff = 0.0;
            for (auto& rawWt : rawWaitingTime[i])
            {
                tot_diff += pow(rawWt - mean_wt, 2);
            }
            waitingTimeVar[i] = tot_diff / rawWaitingTime[i].size();

            // responseTime[i] = (occupancy_buf[i]+occupancy_ser[i]) / throughput[i];
            double mean_rt =
                std::accumulate(rawResponseTime[i].begin(), rawResponseTime[i].end(), 0.0) / rawResponseTime[i].size();
            responseTime[i] = mean_rt;

            tot_diff = 0.0;
            for (auto& rawRt : rawResponseTime[i])
            {
                tot_diff += pow(rawRt - mean_rt, 2);
            }
            responseTimeVar[i] = tot_diff / rawResponseTime[i].size();

            totx += throughput[i];
            totRawWaitingTime.insert(totRawWaitingTime.end(), rawWaitingTime[i].begin(), rawWaitingTime[i].end());
            totRawResponseTime.insert(totRawResponseTime.end(), rawResponseTime[i].begin(), rawResponseTime[i].end());

            preemption_avg[i] = ((double)preemption[i]) / (double)completion[i];
        }

        for (int i = 0; i < occupancy_ser.size(); i++)
            util += occupancy_ser[i] * sizes[i];

        rep_window_size.push_back(std::accumulate(windowSize.begin(), windowSize.end(), 0.0) / simtime);
        rep_preemption.push_back(preemption_avg);

        rep_wait.push_back(waitingTime);
        rep_wait_var.push_back(waitingTimeVar);
        rep_resp.push_back(responseTime);
        rep_resp_var.push_back(responseTimeVar);
        rep_waste.push_back(waste / simtime);
        rep_viol.push_back(viol / simtime);
        rep_util.push_back(util / n);
        rep_tot_buf.push_back(totq);
        // rep_tot_wait.push_back(totq/totx);
        double mean_wt =
            std::accumulate(totRawWaitingTime.begin(), totRawWaitingTime.end(), 0.0) / totRawWaitingTime.size();
        rep_tot_wait.push_back(mean_wt);

        double tot_diff = 0.0;
        for (auto& rawWt : totRawWaitingTime)
        {
            tot_diff += pow(rawWt - mean_wt, 2);
        }
        rep_tot_wait_var.push_back(tot_diff / totRawWaitingTime.size());

        // rep_tot_resp.push_back((totq+tots)/totx);
        double mean_rt =
            std::accumulate(totRawResponseTime.begin(), totRawResponseTime.end(), 0.0) / totRawResponseTime.size();
        rep_tot_resp.push_back(mean_rt);

        tot_diff = 0.0;
        for (auto& rawRt : totRawResponseTime)
        {
            tot_diff += pow(rawRt - mean_rt, 2);
        }
        rep_tot_resp_var.push_back(tot_diff / totRawResponseTime.size());

        // std::cout<<tot_job_seq_dur[0]<<std::endl;
        // std::cout<<job_seq_amount[0]<<std::endl;

        double avg_big_seq_len = (tot_job_seq[1] * 1.0) / job_seq_amount[1];
        double avg_small_seq_len = (tot_job_seq[0] * 1.0) / job_seq_amount[0];
        double avg_big_seq_dur = (tot_job_seq_dur[1] * 1.0) / job_seq_amount[1];
        double avg_small_seq_dur = (tot_job_seq_dur[0] * 1.0) / job_seq_amount[0];

        rep_big_seq_avg_len.push_back(avg_big_seq_len);
        rep_small_seq_avg_len.push_back(avg_small_seq_len);
        rep_big_seq_avg_dur.push_back(avg_big_seq_dur);
        rep_small_seq_avg_dur.push_back(avg_small_seq_dur);
        rep_big_seq_amount.push_back(job_seq_amount[1]);
        rep_small_seq_amount.push_back(job_seq_amount[0]);

        rep_phase_two_duration.push_back((phase_two_duration * 1.0) / job_seq_amount[0]);
        rep_phase_three_duration.push_back((phase_three_duration * 1.0) / job_seq_amount[0]);

        /*std::cout << phase_two_duration << std::endl;
        std::cout << phase_three_duration << std::endl;
        std::cout << phase_two_duration+phase_three_duration << std::endl;
        std::cout << tot_job_seq_dur[0] << std::endl;
        std::cout << "-------------------------------------" << std::endl;*/

        // rep_big_sequences.push_back(avg_seq_len);
        // rep_seq_amount.push_back(seq_amount);
        // rep_seq_max_len.push_back(max_seq_len);
    }

    void simulate(unsigned long nevents, unsigned int repetitions = 1)
    {
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


        double tot_lambda = std::accumulate(l.begin(), l.end(), 0.0);
        std::string out_filename = "Results/logfile_N" + std::to_string(n) + "_" + std::to_string(tot_lambda) + "_W" +
            std::to_string(w) + ".csv";
        remove(out_filename.c_str());
        std::ofstream outputFile_rep(out_filename, std::ios::app);
        std::vector<std::string> headers_rep;
        headers_rep = {"Repetition"};
        for (int ts : sizes)
        {
            headers_rep.push_back("T" + std::to_string(ts) + " Queue");
            headers_rep.push_back("T" + std::to_string(ts) + " MQL");
        }

        headers_rep.push_back("Total Queue");
        headers_rep.push_back("Total MQL");

        if (outputFile_rep.tellp() == 0)
        {
            // Write the headers to the CSV file
            for (const std::string& header : headers_rep)
            {
                outputFile_rep << header << ";";
            }
            outputFile_rep << "\n";
        }
        outputFile_rep.close();
        std::vector<std::string> headers;
        headers = {"Repetition", "Event"};
        for (int ts : sizes)
        {
            headers.push_back("T" + std::to_string(ts) + " Queue");
            headers.push_back("T" + std::to_string(ts) + " Service");
        }

        headers.push_back("Total Queue");
        headers.push_back("Total Service");

        for (int i = 0; i < sizes.size(); ++i)
        {
            headers.push_back("Fel" + std::to_string(i));
        }

        for (int i = 0; i < sizes.size(); ++i)
        {
            headers.push_back("Fel" + std::to_string(i + sizes.size()));
        }

        headers.push_back("Simtime");

        {
            remove(logfile_name.c_str());
            std::ofstream outputFile(logfile_name, std::ios::app);
            if (outputFile.tellp() == 0)
            {
                // Write the headers to the CSV file
                for (const std::string& header : headers)
                {
                    outputFile << header << ";";
                }
                outputFile << "\n";
            }
            outputFile.close();
        }

        for (int rep = 0; rep < repetitions; rep++)
        {

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

            for (unsigned long int k = 0; k < nevents; k++)
            {
                auto itmin = std::ranges::min_element(fel);
                // std::cout << *itmin << std::endl;
                int pos = std::distance(fel.begin(), itmin);
                // std::cout << pos << std::endl;
                collect_statistics(pos);
                // std::cout << "collect" << std::endl;
                if (pos < nclasses)
                { // departure
                    if (this->w == -2 || this->sampling_method != 10)
                    {
                        jobs_inservice[pos].erase(
                            job_fel[pos]); // Remove jobs from in_service (they cannot be in preempted list)
                        rawWaitingTime[pos].push_back(waitTime[job_fel[pos]]);
                        rawResponseTime[pos].push_back(waitTime[job_fel[pos]] + holdTime[job_fel[pos]]);
                        arrTime.erase(job_fel[pos]);
                        waitTime.erase(job_fel[pos]);
                        holdTime.erase(job_fel[pos]);
                    }
                    // std::cout << "before dep" << std::endl;

                    policy->departure(pos, sizes[pos], job_fel[pos]);
                    // std::cout << "dep" << std::endl;
                }
                else
                {
                    auto job_id = k + (nevents * rep);
                    if (this->w == -3)
                    {
                        holdTime[job_id] = this->class_samplers[pos - nclasses]->sample();
                        // std::cout << holdTime[job_id] << std::endl;
                    }
                    policy->arrival(pos - nclasses, sizes[pos - nclasses], job_id);
                    arrTime[job_id] = *itmin;
                    // std::cout << "arr" << std::endl;
                }

                simtime = *itmin;

                resample();
                // std::cout << "resample" << std::endl;
                if (this->w == -3)
                {
                    // std::cout << policy->get_state_buf()[0] << " " << simtime << std::endl;
                    bool added;
                    added = policy->fit_jobs(holdTime, simtime);
                    resample();
                    int idx = 0;
                    while (added)
                    {
                        // std::cout << idx << " " << simtime << std::endl;
                        added = policy->fit_jobs(holdTime, simtime);
                        // std::cout << "added" << std::endl;
                        resample();
                        // std::cout << "resample" << std::endl;
                        idx += 1;
                    }
                    // std::cout << "out" << std::endl;
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

            collect_run_statistics(tot_seq_len, seq_amount);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            rep_timings.push_back(duration);

            std::cout << "Repetition " << std::to_string(rep) << " Done" << std::endl;
            // this->reset_data();
            /*auto sb = policy->get_state_buf();
            for (int i = 0; i<sb.size(); ++i) {
                std::cout << sb[i] << ", ";
            }
            std::cout << std::endl;*/
            std::ofstream outputFile(out_filename, std::ios::app);
            outputFile << rep << ";";
            auto state_buf = policy->get_state_buf();
            for (int i = 0; i < occupancy_buf.size(); i++)
            {
                outputFile << state_buf[i] << ";";
                outputFile << occupancy_buf[i] << ";";
            }
            outputFile << std::accumulate(state_buf.begin(), state_buf.end(), 0.0) << ";";
            outputFile << std::accumulate(occupancy_buf.begin(), occupancy_buf.end(), 0.0) << ";";
            outputFile << "\n";
            outputFile.close();
        }

        // outputFile.close();
        for (auto& x : rep_free_servers_distro)
        {
            x /= simtime;
        }

        /*    std::ofstream outFree("freeserversDistro-nClasses" + std::to_string(this->nclasses) + "-N" +
           std::to_string(this->n) + "-Win" + std::to_string(this->w) + ".csv", std::ios::app); lambda =
           std::accumulate(this->l.begin(), this->l.end(), 0.0);

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

    void produce_statistics(ExperimentStats& stats, const double confidence = 0.05) const
    {
        for (int i = 0; i < nclasses; i++)
        {
            stats.occupancy_buf.push_back(compute_interval_class_student(rep_occupancy_buf, i, confidence));
            stats.occupancy_ser.push_back(compute_interval_class_student(rep_occupancy_ser, i, confidence));
            stats.throughput.push_back(compute_interval_class_student(rep_th, i, confidence));
            stats.wait_time.push_back(compute_interval_class_student(rep_wait, i, confidence));
            stats.wait_time_var.push_back(compute_interval_class_student(rep_wait_var, i, confidence));
            stats.resp_time.push_back(compute_interval_class_student(rep_resp, i, confidence));
            stats.resp_time_var.push_back(compute_interval_class_student(rep_resp_var, i, confidence));
            stats.preemption_avg.push_back(compute_interval_class_student(rep_preemption, i, confidence));

            if (1.0 - stats.throughput[stats.throughput.size() - 1].mean / l[i] > 0.05)
                stats.warnings.push_back(true);
            else
                stats.warnings.push_back(false);
        }
        stats.wasted = compute_interval_student(rep_waste, confidence);
        stats.violations = compute_interval_student(rep_viol, confidence);
        stats.utilisation = compute_interval_student(rep_util, confidence);
        stats.occupancy_tot = compute_interval_student(rep_tot_buf, confidence);
        stats.wait_tot = compute_interval_student(rep_tot_wait, confidence);
        stats.wait_var_tot = compute_interval_student(rep_tot_wait_var, confidence);
        stats.resp_tot = compute_interval_student(rep_tot_resp, confidence);
        stats.resp_var_tot = compute_interval_student(rep_tot_resp_var, confidence);
        stats.timings_tot = compute_interval_student(rep_timings, confidence);
        stats.big_seq_avg_len = compute_interval_student(rep_big_seq_avg_len, confidence);
        stats.small_seq_avg_len = compute_interval_student(rep_small_seq_avg_len, confidence);
        stats.big_seq_avg_dur = compute_interval_student(rep_big_seq_avg_dur, confidence);
        stats.small_seq_avg_dur = compute_interval_student(rep_small_seq_avg_dur, confidence);
        stats.big_seq_amount = compute_interval_student(rep_big_seq_amount, confidence);
        stats.small_seq_amount = compute_interval_student(rep_small_seq_amount, confidence);
        stats.phase_two_dur = compute_interval_student(rep_phase_two_duration, confidence);
        stats.phase_three_dur = compute_interval_student(rep_phase_three_duration, confidence);
        stats.window_size = compute_interval_student(rep_window_size, confidence);
    }


private:
    std::vector<double> l;
    std::vector<double> u;
    std::vector<std::unique_ptr<sampler>> class_samplers;
    std::vector<int> sizes;
    int n;
    int w;
    int nclasses;
    bool debugMode;

    Policy* policy;

    double simtime = 0.0;

    // overall statistics
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

    // statistics for single run
    std::vector<double> occupancy_buf;
    std::vector<double> occupancy_ser;
    std::vector<unsigned long> completion;
    std::vector<unsigned long> preemption;
    std::vector<double> throughput;
    std::vector<double> waitingTime;
    std::vector<double> waitingTimeVar;

    std::unordered_map<long int, double> arrTime;
    std::unordered_map<long int, double> waitTime;
    std::unordered_map<long int, double> holdTime;
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
    // std::vector<std::list<double>> fels;
    // std::list<int> job_ids;
    std::vector<std::unordered_map<long int, double>> jobs_inservice; //[id, time_end]
    std::vector<std::unordered_map<long int, double>> jobs_preempted; //[id, time_left]

    std::string logfile_name;

    std::shared_ptr<std::mt19937_64> generator;
    std::uniform_real_distribution<double> ru{0., 1.};

    int sampling_method;

    double sample_exp(double par) // done
    {
        return -log(ru(*generator)) / par;
    }

    void resample()
    {
        // add arrivals and departures
        if (this->w == -2)
        { // special blocks for serverFilling (memoryful)
            auto stopped_jobs = policy->get_stopped_jobs();
            auto ongoing_jobs = policy->get_ongoing_jobs();
            for (int i = 0; i < nclasses; i++)
            {
                if (fel[i + nclasses] <= simtime)
                { // only update arrival that is executed at the time
                    fel[i + nclasses] = sample_exp(l[i]) + simtime;
                }

                for (auto job_id = stopped_jobs[i].begin(); job_id != stopped_jobs[i].end(); ++job_id)
                {
                    if (jobs_inservice[i].contains(*job_id))
                    { // If they are currently being served: stop them
                        jobs_preempted[i][*job_id] =
                            jobs_inservice[i][*job_id] - simtime; // Save the remaining service time
                        jobs_inservice[i].erase(*job_id);
                        arrTime[*job_id] = simtime;
                        preemption[i]++;
                    }
                }

                long int fastest_job_id;
                double fastest_job_fel = std::numeric_limits<double>::infinity();
                for (auto job_id = ongoing_jobs[i].begin(); job_id != ongoing_jobs[i].end(); ++job_id)
                {
                    if (!jobs_inservice[i].contains(*job_id))
                    { // If they are NOT already in service
                        if (jobs_preempted[i].contains(*job_id))
                        { // See if they were preempted: resume them
                            jobs_inservice[i][*job_id] = jobs_preempted[i][*job_id] + simtime;
                            jobs_preempted[i].erase(*job_id);
                            waitTime[*job_id] = simtime - arrTime[*job_id] + waitTime[*job_id];
                        }
                        else
                        { // or they are just new jobs about to be served for the first time: add them with new service
                          // time
                            double sampled = this->class_samplers[i]->sample();
                            jobs_inservice[i][*job_id] = sampled + simtime;
                            // rawWaitingTime[i].push_back(simtime-arrTime[*job_id]);
                            // arrTime.erase(*job_id); //update waitingTime
                            waitTime[*job_id] = simtime - arrTime[*job_id];
                            holdTime[*job_id] = sampled;
                        }

                        if (jobs_inservice[i][*job_id] < fastest_job_fel)
                        {
                            fastest_job_id = *job_id;
                            fastest_job_fel = jobs_inservice[i][*job_id];
                        }
                    }
                    else
                    { // They are already in service
                        if (jobs_inservice[i][*job_id] < fastest_job_fel)
                        {
                            fastest_job_id = *job_id;
                            fastest_job_fel = jobs_inservice[i][*job_id];
                        }
                    }
                }

                if (jobs_inservice[i].empty())
                { // If no jobs in service for a given class
                    fel[i] = std::numeric_limits<double>::infinity();
                }
                else
                {
                    fel[i] = fastest_job_fel;
                    job_fel[i] = fastest_job_id;
                }
            }
        }
        else if (this->sampling_method != 10)
        { // exponential distro can use the faster memoryless blocks
            auto ongoing_jobs = policy->get_ongoing_jobs();
            int pooled_i;
            for (int i = 0; i < nclasses; i++)
            {
                if (i < nclasses - 1)
                {
                    pooled_i = 0;
                }
                else
                {
                    pooled_i = 1;
                }

                if (fel[i + nclasses] <= simtime)
                { // only update arrival that is executed at the time
                    fel[i + nclasses] = sample_exp(l[i]) + simtime;
                }

                // std::cout << ongoing_jobs[i].size() << std::endl;
                for (long int job_id : ongoing_jobs[i])
                {
                    double sampled;
                    if (this->w == -3)
                    {
                        sampled = holdTime[job_id];
                        policy->insert_completion(this->sizes[i], sampled + simtime);
                    }
                    else
                    {
                        sampled = this->class_samplers[i]->sample();
                    }
                    jobs_inservice[i][job_id] = sampled + simtime;
                    // rawWaitingTime[i].push_back();
                    waitTime[job_id] = simtime - arrTime[job_id];
                    holdTime[job_id] = sampled;
                    // arrTime.erase(job_id); //update waitingTime
                    if (jobs_inservice[i][job_id] < fel[i])
                    {
                        fel[i] = jobs_inservice[i][job_id];
                        job_fel[i] = job_id;
                    }

                    if (last_job < 0)
                    {
                        // std::cout << "sequence " << i << " starting from idle" << " simtime " << simtime <<
                        // std::endl;
                        curr_job_seq[pooled_i] = 1;
                        curr_job_seq_start[pooled_i] = simtime;
                        last_job = pooled_i;
                        if (pooled_i == 0)
                        {
                            // std::cout << "phase three starting from idle" << " simtime " << simtime << std::endl;
                            phase_three_start = simtime;
                            curr_phase = 3;
                        }
                        // std::cout << "-------------------------------------" << std::endl;
                    }
                    else if (last_job == pooled_i)
                    {
                        curr_job_seq[pooled_i] = curr_job_seq[pooled_i] + 1;
                    }
                    else
                    {
                        // std::cout << "sequence " << pooled_i << " starting from sequence " << last_job << " simtime "
                        // << simtime << std::endl;
                        tot_job_seq[last_job] = tot_job_seq[last_job] + curr_job_seq[last_job];
                        // if (last_job == 1 && pooled_i == 0) {
                        //     std::cout << simtime << "  " << curr_job_seq_start[last_job] << std::endl;
                        // }
                        tot_job_seq_dur[last_job] = tot_job_seq_dur[last_job] + simtime - curr_job_seq_start[last_job];
                        job_seq_amount[last_job] = job_seq_amount[last_job] + 1;
                        curr_job_seq[last_job] = 0;
                        curr_job_seq[pooled_i] = 1;
                        curr_job_seq_start[pooled_i] = simtime;
                        // std::cout << tot_job_seq_dur[last_job] << std::endl;
                        last_job = pooled_i;
                        if (pooled_i == 0)
                        {
                            if (policy->get_free_ser() == 0)
                            {
                                // std::cout << "phase two starting from phase one" << " simtime " << simtime <<
                                // std::endl;
                                phase_two_start = simtime;
                                curr_phase = 2;
                            }
                            else
                            {
                                // std::cout << "phase three starting from phase one" << " simtime " << simtime <<
                                // std::endl;
                                phase_three_start = simtime;
                                curr_phase = 3;
                            }
                            // std::cout << "-------------------------------------" << std::endl;
                        }
                        else
                        {
                            // std::cout << "phase one starting from phase three" << " simtime " << simtime <<
                            // std::endl;
                            phase_three_duration += (simtime - phase_three_start);
                            if (add_phase_two)
                            {
                                phase_two_duration += (phase_three_start - phase_two_start);
                                add_phase_two = false;
                            }
                            // std::cout << phase_two_duration+phase_three_duration << std::endl;
                            // std::cout << tot_job_seq_dur[0] << std::endl;
                            // std::cout << (phase_three_duration+phase_two_duration == tot_job_seq_dur[0]) <<
                            // std::endl;
                            if (phase_two_start < 0 && phase_two_duration == 0 &&
                                phase_three_duration < tot_job_seq_dur[0])
                            {
                                // phase_two_duration += (phase_three_start-phase_two_start);
                            }
                            curr_phase = 1;
                            // std::cout << "small abis" << std::endl;
                            // std::cout << policy->get_state_ser()[0] << " " << policy->get_state_buf()[1] <<
                            // std::endl; std::cout << phase_three_start << std::endl; std::cout <<
                            // curr_job_seq_start[0] << std::endl; std::cout << phase_two_start << std::endl; std::cout
                            // << phase_three_start << std::endl; std::cout << phase_two_duration << std::endl;
                            // std::cout << phase_three_duration << std::endl;
                            // std::cout << phase_two_duration+phase_three_duration << std::endl;
                            // std::cout << tot_job_seq_dur[0] << std::endl;
                            // std::cout << "-------------------------------------" << std::endl;
                        }
                    }
                }

                if (jobs_inservice[i].empty())
                { // If no jobs in service for a given class
                    fel[i] = std::numeric_limits<double>::infinity();
                }
                else if (fel[i] <= simtime)
                {
                    fel[i] = std::numeric_limits<double>::infinity();
                    for (auto& job : jobs_inservice[i])
                    {
                        if (job.second < fel[i])
                        {
                            fel[i] = job.second;
                            job_fel[i] = job.first;
                        }
                    }
                }
            }

            if (curr_phase == 2)
            {
                if (this->w > -4 && policy->get_free_ser() > 0)
                {
                    // std::cout << "phase three starting from phase two" << " simtime " << simtime << std::endl;
                    // phase_two_duration += (simtime-phase_two_start);
                    phase_three_start = simtime;
                    curr_phase = 3;
                    add_phase_two = true;
                    // std::cout << phase_two_duration << std::endl;
                    // std::cout << "-------------------------------------" << std::endl;
                }
                else if (this->w <= -4 && policy->prio_big() == true)
                {
                    // std::cout << "phase three starting from phase two" << " simtime " << simtime << std::endl;
                    // phase_two_duration += (simtime-phase_two_start);
                    phase_three_start = simtime;
                    curr_phase = 3;
                    add_phase_two = true;
                    // std::cout << phase_two_duration << std::endl;
                    // std::cout << "-------------------------------------" << std::endl;
                }
            }

            if (policy->get_free_ser() == this->n && last_job >= 0)
            {
                // std::cout << "sequence " << last_job << " ending" << " simtime " << simtime << std::endl;
                tot_job_seq[last_job] = tot_job_seq[last_job] + curr_job_seq[last_job];
                tot_job_seq_dur[last_job] = tot_job_seq_dur[last_job] + simtime - curr_job_seq_start[last_job];
                job_seq_amount[last_job] = job_seq_amount[last_job] + 1;
                curr_job_seq[last_job] = 0;
                // std::cout << tot_job_seq_dur[last_job] << std::endl;
                if (last_job == 0)
                {
                    // std::cout << "phase three ending" << " simtime " << simtime << std::endl;
                    // std::cout << add_phase_two << std::endl;
                    // std::cout << phase_two_start << std::endl;
                    // std::cout << phase_three_start << std::endl;
                    if (curr_phase == 3)
                    {
                        phase_three_duration += (simtime - phase_three_start);
                        if (add_phase_two)
                        {
                            phase_two_duration += (phase_three_start - phase_two_start);
                            add_phase_two = false;
                        }
                        if (phase_two_start < 0 && phase_two_duration == 0 &&
                            phase_two_duration + phase_three_duration < tot_job_seq_dur[0])
                        {
                            // std::cout << "HEHE-------------------------------------" << std::endl;
                            // phase_two_duration += (phase_three_start-phase_two_start);
                        }
                    }
                    else if (curr_phase == 2)
                    {
                        phase_two_duration += (simtime - phase_two_start);
                    }
                    else
                    {
                        std::cout << "WADAW-------------------------------------" << std::endl;
                    }
                    // std::cout << phase_two_duration << std::endl;
                    // std::cout << phase_three_duration << std::endl;
                    // std::cout << phase_two_duration+phase_three_duration << std::endl;
                    // std::cout << tot_job_seq_dur[0] << std::endl;
                    // std::cout << phase_three_duration << std::endl;
                    // std::cout << "-------------------------------------" << std::endl;
                }
                last_job = -1;
                curr_phase = 0;
                // std::cout << "-------------------------------------" << std::endl;
            }

            if (curr_phase == 3)
            {
                // std::cout << policy->get_state_ser()[0] << " " << policy->get_state_buf()[1] << std::endl;
            }
        }
        else
        {
            for (int i = 0; i < nclasses; i++)
            {
                fel[i + nclasses] = sample_exp(l[i]) + simtime;
                if (policy->get_state_ser()[i] > 0)
                {
                    fel[i] = sample_exp((1 / u[i]) * policy->get_state_ser()[i]) + simtime;
                    // fel[i] = sample_st((1/u[i])*policy->get_state_ser()[i]) + simtime;
                    // this can be written in a more static format as
                    // ___exponential::with_mean(u[i])___.sample()/policy->get_state_ser()[i] + simtime;
                    // but it needs to be understood who is the owner of the sampler
                    // and whether it should statically be exponential or not
                }
                else
                {
                    fel[i] = std::numeric_limits<double>::infinity();
                }
            }
        }
    }

    void collect_statistics(int pos)
    {

        if (pos < nclasses)
            completion[pos]++;

        double delta = fel[pos] - simtime;
        for (int i = 0; i < nclasses; i++)
        {
            occupancy_buf[i] += policy->get_state_buf()[i] * delta;
            occupancy_ser[i] += policy->get_state_ser()[i] * delta;
        }
        auto occ = 0;
        for (int i = 0; i < nclasses; i++)
        {
            occ += policy->get_state_ser()[i] * sizes[i];
        }
        waste += (n - occ) * delta;
        viol += policy->get_violations_counter() * delta;
        rep_free_servers_distro[policy->get_free_ser()] += delta;

        windowSize.push_back(policy->get_window_size() * delta);
    }
};

void run_simulation(Experiment e, unsigned long events, unsigned int repetitions,
                    ExperimentStats& stats // out
)
{
    Simulator sim(e.l, e.u, e.s, e.w, e.n, e.sm, e.logf);
    sim.reset_simulation();
    sim.reset_statistics();

    sim.simulate(events, repetitions);
    sim.produce_statistics(stats);
}

int main(int argc, char* argv[])
{

    std::vector<double> p;
    std::vector<int> sizes;
    std::vector<double> mus;
    std::vector<double> arr_rate;
    std::vector<std::string> headers;
    std::vector<double> input_utils;

    std::string cell;
    int n;
    int w;
    int sampling_method;
    std::string type;
    int n_evs;
    int n_runs;
    std::vector<std::string> sampling_name;
    std::string out_filename;
    from_argv(argv, p, sizes, mus, arr_rate, headers, cell, n, w, sampling_method, type, n_evs, n_runs, sampling_name,
              out_filename);
    std::ofstream outputFile(out_filename, std::ios::app);
    std::vector<Experiment> ex;


    for (int i = 0; i < arr_rate.size(); i++)
    {
        std::vector<double> l;
        for (auto x : p)
        {
            l.push_back(x * arr_rate[i]);
        }
        std::string logfile_name = "Results/logfile-nClasses" + std::to_string(sizes.size()) + "-N" +
            std::to_string(n) + "-Win" + std::to_string(w) + "-" + sampling_name[sampling_method] + "-" + cell + "-" +
            "-lambda" + std::to_string(arr_rate[i]) + ".csv";
        ex.push_back(Experiment{l, mus, sizes, w, n, sampling_method, logfile_name});
    }

    std::vector<ExperimentStats> experiments_stats(ex.size());

    std::vector<std::thread> threads(ex.size());

    for (int i = 0; i < ex.size(); i++)
    {
        threads[i] = std::thread(run_simulation, ex[i], n_evs, n_runs, std::ref(experiments_stats[i]));
    }
    for (int i = 0; i < ex.size(); i++)
    {
        threads[i].join();
    }

    if (outputFile.tellp() == 0)
    {
        experiments_stats.at(0).add_headers(headers, sizes);
        // Write the headers to the CSV file
        for (const std::string& header : headers)
        {
            outputFile << header << ";";
        }
        outputFile << "\n";
    }

    for (int i = 0; i < ex.size(); i++)
    {

        outputFile << arr_rate[i] << ";";
        outputFile << experiments_stats[i] << "\n";
    }

    // Close the file
    outputFile.close();


    return 0;
}
