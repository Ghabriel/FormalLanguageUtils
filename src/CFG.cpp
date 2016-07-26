#include <cassert>
#include <queue>
#include <regex>
#include "CFG.hpp"

CFG& CFG::add(const Symbol& name, const BNF& production) {
    assert(!isTerminal(name));
    Production prod(name);
    prod.products = toSymbolSequence(production);
    for (Symbol& symbol : prod.products) {
        if (isTerminal(symbol)) {
            terminals.insert(symbol);
        } else {
            nonTerminals.insert(symbol);
        }
    }
    nonTerminals.insert(name);
    productions.push_back(std::move(prod));
    invalidate();
    return *this;
}

CFG& CFG::add(const CFG::BNF& prodGroup) {
    const static std::regex valid("(<[A-Za-z0-9_']+>) ?::= ?(.*)");
    std::smatch matches;
    std::regex_match(prodGroup, matches, valid);
    assert(matches.size() > 0);
    std::string buffer;
    std::string productions = matches[2];
    for (char c : productions) {
        if (c == '|') {
            add(matches[1], buffer);
            buffer.clear();
        } else {
            buffer += c;
        }
    }
    add(matches[1], buffer);
    return *this;
}

CFG& CFG::operator<<(const CFG::BNF& production) {
    return add(production);
}

void CFG::clear() {
    productions.clear();
    nonTerminals.clear();
    terminals.clear();
    isFirstValid = false;
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
        prodNames.insert(prod.name);
    }
    for (auto& prod : productions) {
        for (auto& symbol : prod.products) {
            if (!isTerminal(symbol) && prodNames.count(symbol) == 0) {
                return false;
            }
        }
    }
    return true;
}

std::unordered_set<CFG::Symbol> CFG::first(const CFG::BNF& symbolSequence) const {
    std::vector<Symbol> symbols = toSymbolSequence(symbolSequence);
    std::unordered_set<Symbol> result;
    updateFirst();
    for (Symbol& symbol : symbols) {
        if (isTerminal(symbol)) {
            result.insert(symbol);
            break;
        }
        select(symbol, [&result](const Production& prod) {
            for (auto& s : prod.firstSet) {
                result.insert(s);
            }
        });
        if (!nullable(symbol)) {
            break;
        }
    }
    return result;
}

bool CFG::nullable(const CFG::BNF& symbolSequence) const {
    std::vector<Symbol> symbols = toSymbolSequence(symbolSequence);
    updateFirst();
    for (Symbol& symbol : symbols) {
        bool quit = true;
        select(symbol, [&](const Production& prod) {
            if (!prod.nullable) {
                quit = false;
            }
        });
        if (quit) {
            return false;
        }
    }
    return true;
}

std::vector<CFG::Symbol> CFG::toSymbolSequence(const CFG::BNF& input) const {
    std::vector<Symbol> result;
    std::string buffer;
    bool record = false;
    for (char c : input) {
        if (c == '<') {
            record = true;
        } else if (c == '>') {
            record = false;
        }
        buffer += c;
        if (!record) {
            result.push_back(buffer);
            buffer.clear();
        }
    }
    return result;
}

bool CFG::isTerminal(const CFG::Symbol& symbol) const {
    return symbol[0] != '<';
}

void CFG::select(const CFG::Symbol& symbol,
    const std::function<void(const CFG::Production&)>& callback) const {
    if (!isTerminal(symbol)) {
        for (auto index : productionsBySymbol.at(symbol)) {
            callback(productions[index]);
        }
    }
}

void CFG::updateFirst() const {
    if (isFirstValid) {
        return;
    }
    std::vector<std::unordered_set<Symbol>> registry;
    std::unordered_map<Symbol, bool> nullability;
    std::unordered_set<std::size_t> finishedNonTerminals;
    std::queue<std::size_t> uncertain;
    nullability.reserve(size());
    registry.resize(size());
    for (std::size_t i = 0; i < size(); i++) {
        const Production& prod = productions[i];
        if (nullability.count(prod.name) == 0) {
            nullability[prod.name] = false;
        }
        if (prod.products.size() == 0) {
            nullability[prod.name] = true;
        } else {
            const Symbol& startingSymbol = prod.products[0];
            if (isTerminal(startingSymbol)) {
                registry[i].insert(startingSymbol);
            } else {
                uncertain.push(i);
            }
        }
    }

    while (!uncertain.empty()) {
        std::size_t index = uncertain.front();
        uncertain.pop();
        const Production& prod = productions[index];
        for (auto& symbol : prod.products) {

        }
    }

    // for (unsigned i = 0; i < size(); i++) {
    //     TRACE(i);
    //     TRACE(productions[i].name);
    //     TRACE(nullability[productions[i].name]);
    //     TRACE_IT(registry[i]);
    //     ECHO("-----");
    // }
    assert(false);
}

void CFG::invalidate() {
    isFirstValid = false;
}
