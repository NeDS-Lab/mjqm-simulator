//
// Created by Marco Ciotola on 12/02/25.
//

#ifndef TUPLE_SER_H
#define TUPLE_SER_H
#include <tuple>

namespace boost::serialization {

    template <uint N>
    struct Serialize {
        template <class Archive, typename... Args>
        static void serialize(Archive& ar, std::tuple<Args...>& t, const unsigned int version) {
            ar& std::get<N - 1>(t);
            Serialize<N - 1>::serialize(ar, t, version);
        }
    };

    template <>
    struct Serialize<0> {
        template <class Archive, typename... Args>
        static void serialize(Archive& ar, std::tuple<Args...>& t, const unsigned int version) {
            (void)ar;
            (void)t;
            (void)version;
        }
    };

    template <class Archive, typename... Args>
    void serialize(Archive& ar, std::tuple<Args...>& t, const unsigned int version) {
        Serialize<sizeof...(Args)>::serialize(ar, t, version);
    }

} // namespace boost::serialization

#endif // TUPLE_SER_H
