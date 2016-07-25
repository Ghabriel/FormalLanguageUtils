/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <functional>
#include <iostream>
#include <unordered_map>

#define TRACE(x) std::cout << (#x) << " = " << (x) << std::endl
#define TRACE_L(x,y) std::cout << (x) << " = " << (y) << std::endl
#define TRACE_IT(x) \
    {\
        unsigned counter = 0; \
        for (auto& elem : (x)) { \
            std::cout << (#x) << "[" << std::to_string(counter++) << "] = " << elem << std::endl; \
        }\
    }
#define ECHO(x) std::cout << (x) << std::endl

namespace utils {
    using Index = long;

    template<typename T1, typename T2>
    class bimap {
    private:
        // using K1 = std::reference_wrapper<const T1>;
        // using K2 = std::reference_wrapper<const T2>;
        using K1 = T1;
        using K2 = T2;
    public:
        void reserve(std::size_t size) {
            ltr.reserve(size);
            rtl.reserve(size);
        }

        void insert(const T1& first, const T2& second) {
            ltr[first] = second;
            rtl[second] = first;
        }

        void erase(const T1& key) {
            auto& value = (*this)[key];
            ltr.erase(key);
            rtl.erase(value);
        }

        void erase(const T2& key) {
            auto& value = (*this)[key];
            ltr.erase(value);
            rtl.erase(key);
        }

        unsigned count(const T1& value) const {
            return ltr.count(value);
        }

        unsigned count(const T2& value) const {
            return rtl.count(value);
        }

        T2& operator[](const T1& value) {
            return ltr.at(value);
        }

        T1& operator[](const T2& value) {
            return rtl.at(value);
        }

        const T2& operator[](const T1& value) const {
            return ltr.at(value);
        }

        const T1& operator[](const T2& value) const {
            return rtl.at(value);
        }

        std::size_t size() const {
            return ltr.size();
        }

        typename std::unordered_map<K1, T2>::iterator begin() {
            return ltr.begin();
        }

        typename std::unordered_map<K1, T2>::const_iterator begin() const {
            return ltr.cbegin();
        }

        typename std::unordered_map<K1, T2>::iterator end() {
            return ltr.end();
        }

        typename std::unordered_map<K1, T2>::const_iterator end() const {
            return ltr.cend();
        }

    private:
        std::unordered_map<K1, T2> ltr;
        std::unordered_map<K2, T1> rtl;
    };
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