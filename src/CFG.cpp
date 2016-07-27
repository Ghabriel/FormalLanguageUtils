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
    productionsBySymbol.clear();
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
    std::unordered_set<ProductionPointer> remaining;

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
            remaining.insert(i);
        }
    }

    using ProdSet = std::unordered_set<ProductionPointer>;
    std::function<void(ProductionPointer, ProdSet&)> populate = [&](ProductionPointer index, ProdSet& visited) {
        if (visited.count(index) > 0) {
            return;
        }
        visited.insert(index);

        remaining.erase(index);
        const Production& prod = productions[index];
        SymbolPointer i = progress[index].second;
        std::size_t numProducts = prod.size();
        while (i < numProducts) {
            const Symbol& symbol = prod[i];
            if (isTerminal(symbol)) {
                prod.firstSet.insert(symbol);
                progress[index].first = true;
                nonTerminalProgress[prod.name]--;
                break;
            }

            bool isNullable = false;
            for (auto prodIndex : productionsBySymbol.at(symbol)) {
                auto& otherProd = productions[prodIndex];
                if (symbol != prod.name) {
                    populate(prodIndex, visited);
                    for (auto& s : otherProd.firstSet) {
                        prod.firstSet.insert(s);
                    }
                }
                if (otherProd.nullable) {
                    isNullable = true;
                }
            }

            if (!isNullable) {
                // We're done.
                progress[index].first = true;
                nonTerminalProgress[prod.name]--;
                break;
            }
            i++;
        }

        if (i == prod.size()) {
            // If the loop ended "naturally", this production
            // can derive the empty string.
            if (!prod.nullable) {
                remaining.clear();
                for (std::size_t j = 0; j < size(); j++) {
                    auto& p = productions[j];
                    if (p.size() != 0 && !isTerminal(p[0])) {
                        remaining.insert(j);
                    }
                }
            }
            prod.nullable = true;
            progress[index].first = true;
            nonTerminalProgress[prod.name]--;
        }
    };

    // Calculates all remaining first sets
    while (!remaining.empty()) {
        ProductionPointer index = *remaining.begin();
        ProdSet visited;
        populate(index, visited);
    }

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
