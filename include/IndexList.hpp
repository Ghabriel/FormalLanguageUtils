#ifndef INDEXLIST_HPP
#define INDEXLIST_HPP

#include <cmath>

namespace {
    using Data = unsigned long long;
}

class IndexList {
public:
    IndexList(std::size_t size) {
        list = std::pow(2, size) - 1;
    }

    void remove(Data index) {
        list ^= (1 << index);
    }

    bool isSet(Data index) {
        return (list & (1 << index)) != 0;
    }

private:
    Data list;
};

#endif
