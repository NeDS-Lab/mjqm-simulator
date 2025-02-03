//
// Created by Marco Ciotola on 21/01/25.
//

#ifndef POLICIES_H
#define POLICIES_H

#include <mjqm-policy/BackFilling.h>
#include <mjqm-policy/MostServerFirst.h>
#include <mjqm-policy/MostServerFirstSkip.h>
#include <mjqm-policy/MostServerFirstSkipThreshold.h>
#include <mjqm-policy/ServerFilling.h>
#include <mjqm-policy/ServerFillingMem.h>
#include <mjqm-policy/Smash.h>
#include <mjqm-policy/policy.h>

enum policies {
    smash,
    server_filling,
    server_filling_mem,
    back_filling,
    most_server_first,
    most_server_first_skip,
    most_server_first_skip_threshold
};

static std::unordered_map<std::string_view, policies> policies_map = {
    {"smash", smash},
    {"server filling", server_filling},
    {"server filling memoryful", server_filling_mem},
    {"back filling", back_filling},
    {"most server first", most_server_first},
    {"most server first skip", most_server_first_skip},
    {"most server first skip threshold", most_server_first_skip_threshold},
};

#endif // POLICIES_H
