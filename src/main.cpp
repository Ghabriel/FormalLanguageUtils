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
    instance << "q4";
    instance << "q5";
    instance.accept("q3");
    instance.addTransition("q0", "q1", 'a');
    instance.addTransition("q0", "q2", 'b');
    instance.addTransition("q1", "q2", 'b');
    instance.addTransition("q2", "q1", 'b');
    instance.addTransition("q1", "q3", 'c');
    instance.addTransition("q2", "q3", 'c');
    instance.addTransition("q3", "q4", 'a');
    instance.addTransition("q4", "q4", 'b');
    instance.addTransition("q5", "q2", 'a');

    DFA minimized = instance.minimized();
    TRACE(minimized.size());
}
