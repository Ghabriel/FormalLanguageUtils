/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include "parsers/SLR1.hpp"

parser::SLR1::SLR1(const CFG& cfg) : Parser(cfg) {
    auto lr0 = parser::LR0(cfg);
    ECHO("NOT YET IMPLEMENTED");
    assert(false);
}

ParseResults parser::SLR1::parse(const std::vector<Token>& tokens) {
    ParseResults results;
    ECHO("NOT YET IMPLEMENTED");
    assert(false);
    return results;
}

bool parser::SLR1::canParse() const {
    return !conflict;
}
