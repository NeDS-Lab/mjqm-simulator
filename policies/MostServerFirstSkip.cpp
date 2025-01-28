//
// Created by mccio on 21/01/25.
//

#include "MostServerFirstSkip.h"

void MostServerFirstSkip::arrival(int c, int size, long int id) {
    state_buf[c]++;
    stopped_jobs[c].push_back(id);
    if (drops_below && big_priority == false && c == state_buf.size() - 1 && state_ser[c] == 0 &&
        get_state_ser_small() > 0) {
        // std::cout << "give to big" << std::endl;
        // std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
        big_priority = true;
    }
    flush_buffer();
}
void MostServerFirstSkip::departure(int c, int size, long int id) {
    // std::cout << servers << std::endl;
    state_ser[c]--;
    // std::cout << "keluar " << c << std::endl;
    freeservers += size;
    if (big_priority && c < state_buf.size() - 1 && freeservers >= sizes[sizes.size() - 1]) {
        // std::cout << "return to big" << std::endl;
        big_priority = false;
        drops_below = false;
    }
    flush_buffer();
    // std::cout << freeservers << " " << state_buf[state_buf.size()-1] << std::endl;
    // std::cout << drops_below << big_priority << std::endl;
    if (big_priority == false && drops_below == false && c < state_buf.size() - 1 && freeservers >= threshold &&
        state_ser[state_ser.size() - 1] == 0) {
        // std::cout << "drops below" << std::endl;
        // std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
        drops_below = true;
        if (state_buf[state_buf.size() - 1] > 0) {
            // std::cout << "give to big" << std::endl;
            // std::cout << get_state_ser()[0] << " " << get_state_buf()[1] << std::endl;
            big_priority = true;
            // std::cout << state_ser[0] << std::endl;
            if (freeservers >= sizes[sizes.size() - 1]) {
                // std::cout << state_ser[1] << std::endl;
                // std::cout << "masuk lwt bawah" << std::endl;
                big_priority = false;
                drops_below = false;
                flush_buffer();
                // std::cout << state_ser[1] << std::endl;
            }
        }
    }
}
int MostServerFirstSkip::get_state_ser_small() {
    int tot_small_ser = 0;
    for (int i = 0; i < state_ser.size() - 1; ++i) {
        tot_small_ser += state_ser[i];
    }
    return tot_small_ser;
}
void MostServerFirstSkip::flush_buffer() {

    ongoing_jobs.clear();
    ongoing_jobs.resize(state_buf.size());

    bool modified = true;
    // bool zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
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
        // zeros = std::all_of(state_buf, state_buf + state_buf.size(), [](bool elem){ return elem == 0; });
    }
}
