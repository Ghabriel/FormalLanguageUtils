#include <iostream>
#include "DFA.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
	return stream << state.getName();
}

int main(int, char**) {
	ECHO("test");
}
