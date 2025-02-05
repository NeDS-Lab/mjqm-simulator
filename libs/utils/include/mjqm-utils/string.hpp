//
// Created by Marco Ciotola on 05/02/25.
//
#ifndef MJQM_UTILS_STRINGS_HPP
#define MJQM_UTILS_STRINGS_HPP

#include <sstream>
#include <string>
#include <string_view>
using std::string_view_literals::operator""sv;

template <typename Iterator, typename Separator = std::string_view>
std::string join(Iterator begin, Iterator end, Separator&& separator = ", "sv) {
    std::ostringstream o;
    if (begin != end) {
        o << *begin++;
        for (; begin != end; ++begin) {
            o << separator << *begin;
        }
    }
    return o.str();
}

#endif // MJQM_UTILS_STRINGS_HPP
