/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef LL1_HPP
#define LL1_HPP

#include <stack>
#include <unordered_map>
#include "Parser.hpp"

namespace parser {
    class LL1 : public Parser {
    public:
        using Parser::Symbol;
        using Parser::TokenType;

        LL1(const CFG&);
        ParseResults parse(const std::vector<Token>&) override;
        bool canParse() const override;

    private:
        std::unordered_map<Symbol, std::unordered_map<TokenType, unsigned>> table;
        bool conflict = false;

        ParseResults unwind(std::stack<Symbol>&, const TokenType&);
        ParseResults error(const std::vector<Token>&, std::size_t, const std::string&) const;
    };
}

#endif
