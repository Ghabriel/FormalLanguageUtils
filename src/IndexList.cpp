#include <cmath>
#include "IndexList.hpp"

IndexList::IndexList(std::size_t size) : size(size) {
    list = std::pow(2, size) - 1;
}

IndexList& IndexList::remove(IndexList::Data index) {
    list ^= (1 << index);
    return *this;
}

bool IndexList::isSet(IndexList::Data index) const {
    return (list & (1 << index)) != 0;
}

IndexList::Data IndexList::count() const {
    Data result = 0;
    Data copy = list;
    while (list > 0) {
        result++;
        copy = copy & (copy - 1);
    }
    return result;
}

IndexList::Data getList() const {
    return list;
}

IndexList IndexList::operator!() const {
    IndexList newList(size);
    newList.list ^= list;
    return newList;
}

IndexList IndexList::operator~() const {
    return !*this;
}

IndexList IndexList::operator&(const IndexList& other) const {
    IndexList newList(std::max(size, other.size));
    newList.list = list & other.list;
}

IndexList IndexList::operator-(const IndexList& other) const {
    return *this & ~other;
}

bool IndexList::operator==(const IndexList& other) const {
    return list == other.list;
}

bool IndexList::operator!=(const IndexList& other) const {
    return list != other.list;
}

bool IndexList::operator==(const Data& data) const {
    return list == data;
}

bool IndexList::operator!=(const Data& data) const {
    return list != data;
}
