/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include "CFG.hpp"

struct ParseResults {
    bool accepted;
    std::size_t errorIndex;
    std::string errorMessage;
};

class Parser {
public:
    using Symbol = std::string;
    Parser(const CFG& cfg) : cfg(cfg) {}
    const CFG& getCFG() const {
        return cfg;
    }

    virtual ParseResults parse(const std::vector<Symbol>&) = 0;
    virtual bool canParse() const = 0;

private:
    CFG cfg;
};

#endif
