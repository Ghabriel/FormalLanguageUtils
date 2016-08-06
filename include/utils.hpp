/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
#include <functional>
#include <iostream>
#include <unordered_map>
#include "utils/composite_iterator.hpp"
#include "utils/bimap.hpp"
#include "utils/reverse_iterator.hpp"

#define TRACE(x) std::cout << (#x) << " = " << (x) << std::endl
#define TRACE_L(x,y) std::cout << (x) << " = " << (y) << std::endl
#define TRACE_IT(x) \
    {\
        unsigned counter = 0; \
        for (auto& elem : (x)) { \
            std::cout << (#x) << "[" << std::to_string(counter++) << "] = " << elem << std::endl; \
        }\
    }
#define TRACE_ITL(l,x) \
    {\
        unsigned counter = 0; \
        for (auto& elem : (x)) { \
            std::cout << (l) << "[" << std::to_string(counter++) << "] = " << elem << std::endl; \
        }\
    }
#define ECHO(x) std::cout << (x) << std::endl
#define ECHOI(x,limit) \
    for (unsigned i = 0; i < limit; i++) {\
        std::cout << "\t"; \
    }\
    ECHO(x);

namespace utils {
    using Index = long;

    template<typename... Args>
    std::string format(const std::string& base, Args&&... args) {
        char* container;
        sprintf(container, base.c_str(), args...);
        return container;
    }
}

// namespace std {
//     template<typename T>
//     struct hash<std::reference_wrapper<T>> {
//         std::size_t operator()(const std::reference_wrapper<T>& value) const {
//             return std::hash<T>()(value);
//         }
//     };

//     template<typename T>
//     struct hash<std::reference_wrapper<const T>> {
//         std::size_t operator()(const std::reference_wrapper<const T>& value) const {
//             return std::hash<T>()(value);
//         }
//     };
// }

// template<typename T1, typename T2>
// inline bool operator==(const std::reference_wrapper<T1>& lhs, const std::reference_wrapper<T2>& rhs) {
//     return lhs.get() == rhs.get();
// }

#endif