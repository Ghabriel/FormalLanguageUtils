/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <fstream>
#include <iostream>
#include <iterator>
#include "CFG.hpp"
#include "DFA.hpp"
#include "Lexer.hpp"
#include "parsers/LL1.hpp"
#include "representations/BNF.hpp"
#include "representations/DidacticNotation.hpp"

int main(int, char**) {
    // std::ifstream stream("tests/test1.txt");
    // std::string input((std::istreambuf_iterator<char>(stream)),
    //                   (std::istreambuf_iterator<char>()));
    // Lexer lexer;
    // lexer.addToken("TYPE", "int|float|double|char|unsigned|string");
    // lexer.addToken("WHILE", "while");
    // lexer.addToken("ASSIGNMENT", "=");
    // lexer.addToken(",", ",");
    // lexer.addToken("(", "\\(");
    // lexer.addToken(")", "\\)");
    // lexer.addToken("{", "\\{");
    // lexer.addToken("}", "\\}");
    // lexer.addToken(";", ";");
    // lexer.addToken("ARITHMETIC_OPERATOR", "\\+|-|\\*|/|%");
    // lexer.addToken("COMPARATOR", "<|>|<=|>=|==");
    // lexer.addToken("BINARY_OPERATORS", "^|&|\\|");
    // lexer.addToken("NUMBER", "[0-9]+\\.?[0-9]*|\\.[0-9]+");
    // lexer.addToken("IDENTIFIER", "[A-Za-z_][A-Za-z0-9_]*");
    // lexer.ignore(' ');
    // lexer.ignore('\n');
    // lexer.ignore('\t');
    // lexer.addDelimiters("[^A-Za-z0-9_.]");

    // TRACE_IT(lexer.read(input));
    // if (!lexer.accepts()) {
    //     TRACE(lexer.getError());
    // } else {
    //     ECHO("OK");
    // }

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

    auto cfg = CFG::create(BNF());
    std::ifstream stream("tests/grammar1_bnf.txt");
    std::string line;
    while (std::getline(stream, line)) {
        cfg << line;
    }
    parser::LL1 parser(cfg);
    if (parser.canParse()) {
        ParseResults results = parser.parse({"id", "+", "+", "id", "*","id"});
        if (results.accepted) {
            ECHO("OK");
        } else {
            // TRACE(results.errorIndex);
            ECHO(results.errorMessage);
        }
    } else {
        ECHO("Error: not a valid LL(1) grammar");
    }
}
