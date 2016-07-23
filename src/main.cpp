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
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q0", 'a');
    instance.accept("q0");
    instance.debug();
    ECHO("-----------------");

    DFA complement;
    complement = ~instance;
    complement.debug();
    TRACE(complement.size());
}
