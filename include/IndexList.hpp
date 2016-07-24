#ifndef INDEXLIST_HPP
#define INDEXLIST_HPP

#include <cmath>
#include <functional>
#include <ostream>
#include <vector>
#include "utils.hpp"

/*
 * A data structure focused on storing sequential values in the range [0, size)
 * in which insertion, search and removal are all O(1) operations. Utilizes
 * ceil(size / 64) lists of 64 values each and uses bit arithmetic to perform
 * all operations.
 */
class IndexList {
public:
    using ull = unsigned long long;
    IndexList(ull);
    IndexList& remove(ull);
    ull extract();
    bool isSet(ull) const;
    ull count() const;
    // ull getList() const;
    std::size_t hash() const;
    IndexList operator!() const;
    IndexList operator~() const;
    IndexList operator&(const IndexList&) const;
    IndexList operator-(const IndexList&) const;
    bool operator==(const IndexList&) const;
    bool operator!=(const IndexList&) const;
    bool operator==(const ull&) const;
    bool operator!=(const ull&) const;

private:
    std::vector<ull> lists;
    ull size;
    const static ull one = 1;
    const static std::size_t limit = 64;

    ull stream(ull) const;
    const ull& find(ull) const;
    ull& find(ull);
    ull offset(ull) const;
};

namespace std {
    template<>
    struct hash<IndexList> {
        std::size_t operator()(const IndexList& list) const {
            // return std::hash<unsigned long long>()(list.getList());
            return list.hash();
        }
    };
}

// inline std::ostream& operator<<(std::ostream& stream, const IndexList& list) {
//  return stream << list.getList();
// }

#endif
