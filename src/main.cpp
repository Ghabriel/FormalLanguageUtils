#include <iostream>
#include "DFA.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    DFA instance;
    instance << "q0";
    instance << "q1";
    instance.addTransition("q0", "q1", 'a');
    instance.accept("q1");

    DFA almostEqual;
    almostEqual << "q0";
    almostEqual << "q1";
    almostEqual.addTransition("q0", "q1", 'b');
    almostEqual.accept("q1");

    TRACE(instance.contains(almostEqual));
    // TRACE(almostEqual.contains(instance));
}
