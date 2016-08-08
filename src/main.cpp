/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <fstream>
#include <iostream>
#include <iterator>
#include "CFG.hpp"
#include "DFA.hpp"
#include "Lexer.hpp"
#include "parsers/LL1.hpp"
#include "parsers/SLR1.hpp"
#include "representations/BNF.hpp"
#include "representations/DidacticNotation.hpp"

int main(int, char**) {
    Lexer lexer;
    // lexer.addToken("SYMBOL", ".");
    lexer.addToken("a", "a");
    lexer.addToken("b", "b");
    lexer.addDelimiters(".");

    auto tokens = lexer.read("aaabbb");
    TRACE_IT(tokens);

    auto cfg = CFG::create(BNF());
    cfg << "<S> ::= 'a' <S> 'b' | 'a' 'b'";
    parser::SLR1 parser(cfg);
    if (parser.canParse()) {
        ParseResults results = parser.parse(tokens);
        if (results.accepted) {
            ECHO("OK");
        } else {
            // TRACE(results.errorIndex);
            ECHO(results.errorMessage);
        }
    } else {
        ECHO("Error: not a valid SLR(1) grammar");
    }


    // std::ifstream stream("tests/test1.txt");
    // std::string input((std::istreambuf_iterator<char>(stream)),
    //                   (std::istreambuf_iterator<char>()));
    // stream.close();

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

    // auto tokens = lexer.read(input);
    // TRACE_IT(tokens);
    // if (!lexer.accepts()) {
    //     TRACE(lexer.getError());
    // } else {
    //     ECHO("OK");
    // }

    // auto cfg = CFG::create(BNF());
    // stream = std::ifstream("tests/grammar1_bnf.txt");
    // std::string line;
    // while (std::getline(stream, line)) {
    //     if (!line.empty()) {
    //         cfg << line;        
    //     }
    // }
    // parser::LL1 parser(cfg);
    // if (parser.canParse()) {
    //     ParseResults results = parser.parse(tokens);
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
