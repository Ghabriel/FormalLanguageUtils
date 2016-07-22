#ifndef INDEXLIST_HPP
#define INDEXLIST_HPP

#include <cmath>

class IndexList {
public:
    IndexList(std::size_t size) {
        list = std::pow(2, size) - 1;
    }

    void remove(unsigned long long index) {
        list ^= (1 << index);
    }

private:
    unsigned long long list;
};

#endif
