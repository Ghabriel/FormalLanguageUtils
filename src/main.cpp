/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <iostream>
#include "CFG.hpp"
#include "DFA.hpp"
#include "DidacticNotation.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    auto test = CFG::create(DidacticNotation());
    test << "S -> a A b | ";
    test << "A -> a A | b A | ";
}
