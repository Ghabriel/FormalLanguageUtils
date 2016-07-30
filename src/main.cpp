/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <iostream>
#include "CFG.hpp"
#include "DFA.hpp"
#include "parsers/LL1.hpp"

int main(int, char**) {
    CFG cfg;
    cfg << "<E> ::= <T><E1>";
    cfg << "<E1> ::= +<T><E1>|";
    cfg << "<T> ::= <F><T1>";
    cfg << "<T1> ::= *<F><T1>|";
    cfg << "<F> ::= (<E>)|i";
    parser::LL1 parser(cfg);
    ParseResults results = parser.parse({"i", "+", "i", "*", "*", "i"});
    if (results.accepted) {
        ECHO("OK");
    } else {
        // TRACE(results.errorIndex);
        ECHO(results.errorMessage);
    }
}
