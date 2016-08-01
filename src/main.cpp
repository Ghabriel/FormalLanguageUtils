/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <iostream>
#include "CFG.hpp"
#include "DFA.hpp"
#include "Lexer.hpp"
#include "parsers/LL1.hpp"
#include "representations/DidacticNotation.hpp"
#include "Regex.hpp"

int main(int, char**) {
    Regex regex("a*b*");
    TRACE(regex.matches("abbaba"));

    // Regex regex("(a|b)*c(de)*");
    // TRACE(regex.matches("abbcdede"));

    // Lexer lexer;
    // lexer.addToken("T_NUMBER", "[0-9]+\\.?[0-9]*|\\.[0-9]+");
    // lexer.addToken("T_PLUS", "\\+");
    // lexer.addToken("T_TIMES", "\\*");
    // lexer.ignore(' ');
    // TRACE_IT(lexer.read("22 3.14 + * 7 + 9"));

    // CFG cfg;
    // cfg << "<E> ::= <T><E1>";
    // cfg << "<E1> ::= +<T><E1>|";
    // cfg << "<T> ::= <F><T1>";
    // cfg << "<T1> ::= *<F><T1>|";
    // cfg << "<F> ::= (<E>)|i";
    // auto cfg = CFG::create(DidacticNotation());
    // cfg << "E -> T E1";
    // cfg << "E1 -> + T E1 | ";
    // cfg << "T -> F T1";
    // cfg << "T1 -> * F T1 | ";
    // cfg << "F -> ( E ) | id";
    // parser::LL1 parser(cfg);
    // if (parser.canParse()) {
    //     ParseResults results = parser.parse({"id", "+", "+", "id", "*","id"});
    //     if (results.accepted) {
    //         ECHO("OK");
    //     } else {
    //         // TRACE(results.errorIndex);
    //         ECHO(results.errorMessage);
    //     }
    // } else {
    //     ECHO("Error: not a valid LL(1) grammar");
    // }
}
