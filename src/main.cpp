/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <iostream>
#include "CFG.hpp"
#include "DFA.hpp"

std::ostream& operator<<(std::ostream& stream, const State& state) {
    return stream << state.getName();
}

int main(int, char**) {
    CFG cfg;
    cfg << "<S> ::= s<A><B><C>|";
    cfg << "<A> ::= a<B><C><S>|";
    cfg << "<B> ::= b<C><S><A>|";
    cfg << "<C> ::= c<S><A><B>|";
    for (auto& symbol : cfg.getNonTerminals()) {
        TRACE_ITL("first(" + symbol + ")", cfg.first(symbol));
        TRACE_L("nullable(" + symbol + ")", cfg.nullable(symbol));
        ECHO("");
    }
    ECHO("###########################################");
    for (auto& symbol : cfg.getNonTerminals()) {
        TRACE_ITL("follow(" + symbol + ")", cfg.follow(symbol));
        TRACE_L("endable(" + symbol + ")", cfg.endable(symbol));
        ECHO("");
    }
}
