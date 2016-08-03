/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef BNF_HPP
#define BNF_HPP

#include <regex>
#include "CFGRepresentation.hpp"
#include "utils.hpp"

class BNF : public CFGRepresentation {
public:
    using CFGRepresentation::decompose;

    bool isTerminal(const std::string& symbol) const override {
        // return (symbol[0] == '"' || symbol[0] == '\'') && symbol[1] == symbol[0];
        return !isNonTerminal(symbol);
    }

    bool isNonTerminal(const std::string& symbol) const override {
        return symbol.front() == '<' && symbol.back() == '>';
    }

    std::vector<ProductionParts> decompose(const std::string& group) const override {
        const static std::regex valid("(<[A-Za-z0-9_']+>) ?::= ?(.*)");
        std::smatch matches;
        std::regex_match(group, matches, valid);
        assert(matches.size() > 0);
        std::string buffer;
        std::string productions = matches[2];
        std::vector<ProductionParts> result;
        for (char c : productions) {
            if (c == '|') {
                result.push_back(decompose(matches[1], buffer));
                buffer.clear();
            } else {
                buffer += c;
            }
        }
        result.push_back(decompose(matches[1], buffer));
        return result;
    }

    std::vector<std::string> toSymbolSequence(const std::string& input) const override {
        TRACE(input);
        std::vector<std::string> result;
        std::stack<char> openings;
        std::string buffer;
        for (char c : input) {
            if ((c == '"' || c == '\'' || c == '<') && openings.empty()) {
                openings.push(c);
                continue;
            }

            if (!openings.empty()) {
                if (((c == '"' || c == '\'') && c == openings.top())
                    || (c == '>' && openings.top() == '<')) {

                    buffer = openings.top() + buffer + c;
                    // if (c == '>') {
                    //     buffer = '<' + buffer + '>';
                    // }

                    if (!buffer.empty()) {
                        result.push_back(std::move(buffer));
                        buffer = "";
                    }
                    openings.pop();
                } else {
                    buffer += c;
                }
                continue;
            }

            assert(c == ' ');
        }
        return result;
    }

    std::string toReadableForm(const std::string& name,
        const std::vector<std::string>& products) const override {

        std::string result = name + " ::= ";
        for (auto& symbol : products) {
            result += symbol;
        }
        return result;
    }

    std::string name(const std::string& symbol) const {
        if (isTerminal(symbol)) {
            return symbol;
        }
        return symbol.substr(1, symbol.size() - 2);
    }
};

#endif