#include <iostream>
#include "DFA.hpp"
#include "IndexList.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    DFA instance;
    instance << "q0";
    instance << "q1";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q0", 'a');
    instance.accept("q0");

    DFA other;
    other << "q0";
    other << "q1";
    other << "q2";
    other << "q3";
    other.addTransition("q0", "q1", 'a');
    other.addTransition("q1", "q2", 'a');
    other.addTransition("q2", "q3", 'a');
    other.addTransition("q3", "q0", 'a');
    other.accept("q0", "q2");

    TRACE(instance == other);
}
