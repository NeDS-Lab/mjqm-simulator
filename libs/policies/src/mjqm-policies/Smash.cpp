//
// Created by Marco Ciotola on 21/01/25.
//

#include <mjqm-policies/Smash.h>

void Smash::arrival(int c, int size, long int id) {
    std::tuple<int, int, long int> e(c, size, id);
    this->buffer.push_back(e);
    state_buf[c]++;
    flush_buffer();
}
void Smash::departure(int c, int size, long int id) {
    state_ser[c]--;
    freeservers += size;
    flush_buffer();
}
void Smash::flush_buffer() {
    ongoing_jobs.clear();
    ongoing_jobs.resize(state_buf.size());

    bool modified = true;
    while (modified && buffer.size() > 0 && freeservers > 0) {
        auto it = buffer.begin();
        auto max = buffer.end();
        int i = 0;
        modified = false;

        while (it != buffer.end() && (i < w || w == 0)) { // find maximum
            if (std::get<1>(*it) <= freeservers && (max == buffer.end() || std::get<1>(*it) > std::get<1>(*max))) {
                max = it;
            }
            ++i;
            ++it;
        }

        if (max != buffer.end()) {
            freeservers -= std::get<1>(*max);
            state_buf[std::get<0>(*max)]--;
            state_ser[std::get<0>(*max)]++;
            ongoing_jobs[std::get<0>(*max)].push_back(std::get<2>(*max));
            if (buffer.begin() != max) {
                violations_counter++;
            }
            buffer.erase(max);
            modified = true;
        }
    }
}
