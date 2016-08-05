#include <iostream>
#include <thread>
#include <vector>

const unsigned limit = 1e8;

void f() { 
    double result = 0;
    for (unsigned i = 1; i < limit; i++) {
        result += 1.0 / i;
    }
    std::cout << "[f] result = " << result << std::endl;
}

void g() {
    double result = 0;
    for (unsigned i = 1; i < limit; i++) {
        result += (i + 1) / i;
    }
    std::cout << "[g] result = " << result << std::endl;
}

void h() {
    double result = 0;
    for (unsigned i = 0; i < limit; i++) {
        if (i % 3 > 0) {
            result += i;
        }
    }
    std::cout << "[h] result = " << result << std::endl;
}

void setup(std::vector<std::thread>& container) {}

template<typename T, typename... Args>
void setup(std::vector<std::thread>& container, const T& fn, Args&&... args) {
    container.emplace_back(fn);
    setup(container, args...);
}

template<typename... Args>
void concurrent(Args&&... args) {
    std::vector<std::thread> container;
    setup(container, args...);
    for (auto& thread : container) {
        thread.join();
    }
}

int main(int, char**) {
    // f();
    // g();
    // h();
    concurrent(f, g, h);
    std::cout << "done" << std::endl;
}