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
        LL1(const CFG&);
        ParseResults parse(const std::vector<Symbol>&) override;
        bool canParse() const override;

    private:
        std::unordered_map<Symbol, std::unordered_map<Symbol, unsigned>> table;
        bool conflict = false;

        ParseResults unwind(std::stack<Symbol>&, const Symbol&);
        ParseResults error(const std::vector<Symbol>&, std::size_t, const std::string&) const;
    };
}

#endif
