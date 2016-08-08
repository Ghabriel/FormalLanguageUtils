/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef SLR1_HPP
#define SLR1_HPP

#include <unordered_map>
#include <vector>
#include "Parser.hpp"

namespace parser {
    struct AscendingAction {
        Action action = Action::UNKNOWN;
        std::size_t target;
    };

    class SLR1 : public Parser {
    public:
        using Parser::Symbol;
        using Parser::TokenType;

        SLR1(const CFG&);
        ParseResults parse(const std::vector<Token>&) override;
        bool canParse() const override;

    private:
        std::unordered_map<std::size_t, std::unordered_map<TokenType, AscendingAction>> table;
        bool conflict = false;
    };
}

#endif
