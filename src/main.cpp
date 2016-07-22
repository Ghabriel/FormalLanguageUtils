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
    instance << "q3";
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q2", 'c');
    instance.addTransition("q3", "q3", 'd');
    TRACE(instance.size());
    DFA other = instance.withoutDeadStates();
    TRACE(other.size());
}
