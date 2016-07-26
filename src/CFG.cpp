#include <cassert>
#include "CFG.hpp"

CFG& CFG::add(const Symbol& name, const BNF& production) {
    Production prod(name);
    std::string buffer;
    bool record = false;
    bool isTerminal;
    for (char c : production) {
        isTerminal = true;
        if (c == '<') {
            record = true;
        } else if (c == '>') {
            record = false;
            isTerminal = false;
        } else {
            buffer += c;
        }
        if (!record) {
            if (isTerminal) {
                prod.products.push_back(buffer);
                terminals.insert(buffer);
            } else {
                prod.products.push_back('<' + buffer + '>');
                nonTerminals.insert(buffer);
            }
            buffer.clear();
        }
    }
    nonTerminals.insert(name);
    productions.push_back(std::move(prod));
    return *this;
}

CFG& CFG::add(const CFG::BNF& production) {
    assert(false);
    return *this;
}

CFG& CFG::operator<<(const CFG::BNF& production) {
    return add(production);
}

std::size_t CFG::size() const {
    return productions.size();
}

std::unordered_set<CFG::Symbol> CFG::getNonTerminals() const {
    return nonTerminals;
}

std::unordered_set<CFG::Symbol> CFG::getTerminals() const {
    return terminals;
}

bool CFG::isConsistent() const {
    std::unordered_set<Symbol> prodNames;
    prodNames.reserve(size());
    for (auto& prod : productions) {
        prodNames.insert('<' + prod.name + '>');
    }
    for (auto& prod : productions) {
        for (Symbol symbol : prod.products) {
            if (symbol[0] == '<' && prodNames.count(symbol) == 0) {
                return false;
            }
        }
    }
    return true;
}
