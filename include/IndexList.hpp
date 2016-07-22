#ifndef INDEXLIST_HPP
#define INDEXLIST_HPP

#include <cmath>

namespace {
    using Data = unsigned long long;
}

class IndexList {
public:
    IndexList(std::size_t size) : size(size) {
        list = std::pow(2, size) - 1;
    }

    void remove(Data index) {
        list ^= (1 << index);
    }

    bool isSet(Data index) const {
        return (list & (1 << index)) != 0;
    }

    IndexList operator!() const {
        IndexList newList(size);
        newList.list ^= list;
        return newList;
    }

private:
    Data list;
    std::size_t size;
};

#endif
