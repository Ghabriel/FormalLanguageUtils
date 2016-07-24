#include <cassert>
#include <cmath>
#include "IndexList.hpp"

IndexList::IndexList(ull size) : size(size) {
    if (size > 0) {
        ull numLists = ((size - 1) >> 6) + 1;
        for (ull i = 0; i < numLists - 1; i++) {
            lists.push_back(stream(limit));
            // lists[i] = stream(limit);
        }
        lists.push_back(stream(1 + ((size - 1) % limit)));
        // lists[numLists - 1] = stream(1 + ((size - 1) % limit));
    }
}

IndexList& IndexList::remove(IndexList::ull index) {
    find(index) ^= offset(index);
    return *this;
}

IndexList::ull IndexList::extract() {
    for (ull i = 0; i < lists.size(); i++) {
        if (lists[i] != 0) {
            return (i << 6) + floor(log2(lists[i]));
        }
    }
    assert(false);
}

bool IndexList::isSet(IndexList::ull index) const {
    return (find(index) & offset(index)) != 0;
}

IndexList::ull IndexList::count() const {
    ull result = 0;
    for (auto copy : lists) {
        while (copy > 0) {
            result++;
            copy = copy & (copy - 1);
        }
    }
    return result;
}

std::size_t IndexList::hash() const {
    return std::hash<ull>()(lists.back());
}

IndexList IndexList::operator!() const {
    IndexList newList(size);
    for (ull i = 0; i < lists.size(); i++) {
        newList.lists[i] ^= lists[i];
    }
    return newList;
}

IndexList IndexList::operator~() const {
    return !*this;
}

IndexList IndexList::operator&(const IndexList& other) const {
    IndexList newList(std::max(size, other.size));
    for (ull i = 0; i < newList.lists.size(); i++) {
        if (i >= lists.size() || i >= other.lists.size()) {
            newList.lists[i] = 0;
        } else {
            newList.lists[i] = lists[i] & other.lists[i];
        }
    }
    return newList;
}

IndexList IndexList::operator-(const IndexList& other) const {
    return *this & ~other;
}

bool IndexList::operator==(const IndexList& other) const {
    if (size != other.size) return false;
    for (ull i = 0; i < lists.size(); i++) {
        if (lists[i] != other.lists[i]) return false;
    }
    return true;
}

bool IndexList::operator!=(const IndexList& other) const {
    return !(*this == other);
}

bool IndexList::operator==(const ull& data) const {
    assert(false);
    // return list == data;
}

bool IndexList::operator!=(const ull& data) const {
    assert(false);
    // return list != data;
}

IndexList::ull IndexList::stream(ull size) const {
    return (one << size) - 1;
}

const IndexList::ull& IndexList::find(ull index) const {
    assert(index < size);
    // ECHO("----- " + std::to_string(size));
    // ECHO("----- " + std::to_string(index));
    // ECHO("----- " + std::to_string(lists.size()));
    // ECHO("----- " + std::to_string(index >> 6));
    // ECHO("#####");
    return lists.at(index >> 6);
}

IndexList::ull& IndexList::find(ull index) {
    assert(index < size);
    // ECHO("----- " + std::to_string(size));
    // ECHO("----- " + std::to_string(index));
    // ECHO("----- " + std::to_string(lists.size()));
    // ECHO("----- " + std::to_string(index >> 6));
    // ECHO("#####");
    return lists.at(index >> 6);
}

IndexList::ull IndexList::offset(ull index) const {
    return one << (index % limit);
}
