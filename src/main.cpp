/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <iostream>
#include "CFG.hpp"
#include "DFA.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    ECHO("main");
}
