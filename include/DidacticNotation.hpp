/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef DIDACTIC_NOTATION_HPP
#define DIDACTIC_NOTATION_HPP

#include <regex>
#include "CFGRepresentation.hpp"
#include "utils.hpp"

class DidacticNotation : public CFGRepresentation {
public:
    using CFGRepresentation::decompose;

    bool isTerminal(const std::string& symbol) const override {
        return !isNonTerminal(symbol);
    }

    bool isNonTerminal(const std::string& symbol) const override {
        return symbol[0] >= 'A' && symbol[0] <= 'Z';
    }

    std::vector<ProductionParts> decompose(const std::string& group) const override {
        ECHO("A");
        const static std::regex valid("([A-Z][^ ]*) ?-> ?(.*)");
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
        std::vector<std::string> result;
        std::string buffer;
        for (char c : input) {
            if (c == ' ') {
                if (buffer.size() > 0) {
                    result.push_back(buffer);
                    buffer.clear();
                }
                continue;
            }
            buffer += c;
        }
        if (buffer.size() > 0) {
            result.push_back(buffer);
        }
        return result;
    }

    std::string toReadableForm(const std::string& name,
        const std::vector<std::string>& products) const override {

        std::string result = name + " -> ";
        for (auto& symbol : products) {
            result += symbol;
        }
        return result;
    }

    std::string name(const std::string& symbol) const {
        return symbol;
    }
};

#endif