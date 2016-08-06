#include <functional>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <vector>

template<typename T, template<typename...> class C>
class composite_iterator {
public:
    template<typename... Args>
    void init(const C<T>& iterable, Args&&... args) {
        iterables.push_back(iterable);
        init(args...);
    }

    void init() {
        index = 0;
        currValue = iterables[index].get().begin();
    }

    auto begin() {
        init();
        return *this;
    }

    auto end() {
        index = iterables.size() - 1;
        currValue = iterables[index].get().end();
        return *this;
    }

    composite_iterator<T, C>& operator++() {
        ++currValue;
        if (currValue == iterables[index].get().end() && index < iterables.size() - 1) {
            index++;
            currValue = iterables[index].get().begin();
        }
        return *this;
    }

    const T& operator*() {
        return *currValue;
    }

    bool operator==(const composite_iterator<T, C>& other) const {
        return index == other.index && currValue == other.currValue;
    }

    bool operator!=(const composite_iterator<T, C>& other) const {
        return !(*this == other);
    }

private:
    std::vector<std::reference_wrapper<const C<T>>> iterables;
    std::size_t index;
    typename C<T>::const_iterator currValue;
};

template<typename T, template<typename...> class C, typename... Args>
auto make_composite(const C<T>& iterable, Args&&... args) {
    composite_iterator<T, C> instance;
    instance.init(iterable, args...);
    return instance;
}

int main(int, char**) {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b{4, 5, 6};
    std::vector<int> c{7, 8, 9};
    for (int i : make_composite(a, b, c)) {
        std::cout << i << std::endl;
    }
}
