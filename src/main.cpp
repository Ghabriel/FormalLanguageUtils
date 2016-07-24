#include <iostream>
#include "DFA.hpp"
#include "IndexList.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    DFA instance;
    auto fn = [](unsigned i) -> std::string {
        return std::to_string(i);
    };

    unsigned limit = 43;
    instance.reserve(limit);
    for (unsigned i = 0; i < limit; i++) {
        instance << ("q" + fn(i));
    }

    instance.accept("q" + fn(limit - 2));
    instance.accept("q" + fn(limit - 1));
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q0", "q2", 'b');
    for (unsigned i = 1; i < limit - 2; i += 2) {
        unsigned next = (i + 1) % limit;
        instance.addTransition("q" + fn(i), "q" + fn(next), 'a');
        instance.addTransition("q" + fn(next), "q" + fn(i), 'a');
        instance.addTransition("q" + fn(i), "q" + fn((i + 2) % limit), 'b');
        instance.addTransition("q" + fn(next), "q" + fn((i + 3) % limit), 'b');
    }

    instance.debug();
    ECHO("------------------------");
    (~instance).debug();
}
