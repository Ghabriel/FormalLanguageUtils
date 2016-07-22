#ifndef INDEXLIST_HPP
#define INDEXLIST_HPP

#include <cmath>
#include <cstdlib>

class IndexList {
private:
    using Data = unsigned long long;
public:
    IndexList(std::size_t);
    IndexList& remove(Data);
    bool isSet(Data) const;
    Data count() const;
    Data getList() const;
    IndexList operator!() const;
    IndexList operator~() const;
    IndexList operator&(const IndexList&) const;
    IndexList operator-(const IndexList&) const;
    bool operator==(const IndexList&) const;
    bool operator!=(const IndexList&) const;
    bool operator==(const Data&) const;
    bool operator!=(const Data&) const;

private:
    Data list;
    std::size_t size;
};

namespace std {
    template<>
    struct hash<IndexList> {
        std::size_t operator()(const IndexList& list) const {
            return std::hash<unsigned long long>()(list.getList());
        }
    };
}

#endif
