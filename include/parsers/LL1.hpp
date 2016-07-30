/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef LL1_HPP
#define LL1_HPP

#include <unordered_map>
#include "Parser.hpp"

namespace parser {
    class LL1 : public Parser {
    public:
        using Symbol = std::string;
        LL1(const CFG&);
        ParseResults parse(const std::string&) override;
        bool canParse() const override;

    private:
        std::unordered_map<Symbol, std::unordered_map<Symbol, unsigned>> table;
        bool conflict = false;
    };
}

#endif
