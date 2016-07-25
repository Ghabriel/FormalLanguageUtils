/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <iostream>
#include "DFA.hpp"
#include "IndexList.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    ECHO("main");
}
