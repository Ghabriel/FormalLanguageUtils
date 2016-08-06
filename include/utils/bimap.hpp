/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#ifndef BIMAP_HPP
#define BIMAP_HPP

#include <unordered_map>

namespace utils {
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

#endif
