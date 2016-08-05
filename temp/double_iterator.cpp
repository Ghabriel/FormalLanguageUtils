#include <iostream>
#include <vector>

template<template<typename T> typename C, typename... Args>
class composite_iterator {
public:
    composite_iterator(const C<T>& iterable, Args&&... args) {
        init(iterable, args...);
    }

    void init(const C<T>& iterable, Args&&... args) {
        iterables.push_back(std::make_pair(iterable.begin(), iterable.end()));
        init(args...);
    }

    void init() {
        currValue = iterables[index].first;
    }

    auto begin() {
        return iterables[0].first;
    }

    composite_iterator<C<T>>& operator++() {
        ++currValue;
        if (currValue == iterables[index].second && index < iterables.size() - 1) {
            index++;
            currValue = iterables[index].first;
        }
    }

    T& operator*() {
        return *currValue;
    }

private:
    std::vector<std::pair<T::iterator, T::iterator>> iterables;
    std::size_t index = 0;
    C<T>::iterator currValue;
};

int main(int, char**) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    for (int i : composite_iterator(a, b)) {
        std::cout << i << std::endl;
    }
}
