#include <iostream>
#include "DFA.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
	return stream << state.getName();
}

int main(int, char**) {
	DFA instance;
    instance << "q0";
    instance << "q1";
    instance << "q2";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'a');
    instance.addTransition("q2", "q0", 'a');
    instance.accept("q0");

    DFA second;
    second << "q0";
    second << "q1";
    second.addTransition("q0", "q1", 'a');
    second.addTransition("q1", "q0", 'a');
    second.accept("q0");

    DFA intersection = instance & second;
    intersection.debug();
}
