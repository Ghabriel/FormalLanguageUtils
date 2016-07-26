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
    productionsBySymbol[name].push_back(size());
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
        bool found = false;
        select(symbol, [&](const Production& prod) {
            if (prod.nullable) {
                found = true;
            }
        });
        if (!found) {
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
    using Index = std::size_t;
    using ProductionPointer = Index;
    using SymbolPointer = Index;
    std::unordered_map<ProductionPointer, std::pair<bool, SymbolPointer>> progress;
    std::unordered_map<Symbol, std::size_t> nonTerminalProgress;
    std::queue<ProductionPointer> remaining;

    auto ntInfo = [&](const Symbol& name) {
        bool nullable = false;
        std::unordered_set<Symbol> result;
        for (auto index : productionsBySymbol.at(name)) {
            auto& prod = productions[index];
            if (prod.nullable) {
                nullable = true;
            }
            for (auto& symbol : prod.firstSet) {
                result.insert(symbol);
            }
        }
        return std::make_pair(nullable, result);
    };

    for (auto& prod : productions) {
        if (nonTerminalProgress.count(prod.name) == 0) {
            nonTerminalProgress[prod.name] = 0;
        }
        nonTerminalProgress[prod.name]++;
    }

    // Calculates trivial first sets
    for (ProductionPointer i = 0; i < size(); i++) {
        const Production& prod = productions[i];
        prod.firstSet.clear();
        prod.nullable = false;
        progress[i] = {false, 0};

        if (prod.size() == 0) {
            prod.nullable = true;
            progress[i].first = true;
            nonTerminalProgress[prod.name]--;
        } else {
            const Symbol& symbol = prod[0];
            if (isTerminal(symbol)) {
                prod.firstSet.insert(symbol);
                progress[i].first = true;
                nonTerminalProgress[prod.name]--;
            }
        }

        if (!progress[i].first) {
            remaining.push(i);
        }
    }

    // Calculates all remaining first sets
    while (!remaining.empty()) {
        ProductionPointer index = remaining.front();
        remaining.pop();
        const Production& prod = productions[index];
        SymbolPointer i = progress[index].second;
        for (; i < prod.size(); i++) {
            const Symbol& symbol = prod[i];
            if (isTerminal(symbol)) {
                prod.firstSet.insert(symbol);
                progress[index].first = true;
                nonTerminalProgress[prod.name]--;
                break;
            }

            auto info = ntInfo(symbol);
            bool isNullable = info.first;
            auto& set = info.second;

            if (nonTerminalProgress[symbol] > 0) {
                if (!isNullable || symbol != prod.name) {
                    // We are not sure if symbol derives the empty
                    // string, so we can't continue.
                    remaining.push(index);
                    progress[index].second = i;
                    break;
                }

                if (symbol == prod.name) {
                    // symbol derives the empty string, so just skip it.
                    continue;
                }
            }

            // We found a symbol which has a complete first set.
            // Push its first set into the current one.
            for (auto& s : set) {
                prod.firstSet.insert(s);
            }

            if (!isNullable) {
                // We're done.
                progress[index].first = true;
                nonTerminalProgress[prod.name]--;
                break;
            }
        }

        if (i == prod.size()) {
            // If the loop ended "naturally", this production
            // can derive the empty string.
            prod.nullable = true;
            progress[index].first = true;
            nonTerminalProgress[prod.name]--;
        }
    }

    // ECHO("####################");
    // for (auto& pair : productionsBySymbol) {
    //     bool nullable = false;
    //     std::unordered_set<Symbol> result;
    //     for (auto index : pair.second) {
    //         auto& prod = productions[index];
    //         if (prod.nullable) {
    //             nullable = true;
    //         }
    //         for (auto& symbol : prod.firstSet) {
    //             result.insert(symbol);
    //         }
    //     }
    //     TRACE(pair.first);
    //     TRACE(nullable);
    //     // TRACE(nonTerminalProgress[pair.first]);
    //     TRACE_IT(result);
    //     ECHO("-----");
    // }
    // assert(false);
    isFirstValid = true;
}

void CFG::invalidate() {
    isFirstValid = false;
}

std::string CFG::toBNF(const Production& prod) const {
    std::string result = prod.name + " ::= ";
    for (auto& symbol : prod.products) {
        result += symbol;
    }
    return result;
}
