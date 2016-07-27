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

    // Just to improve readability.
    const bool NULLABLE = true;

    // Stores information about all non-terminals for which a definitive
    // conclusion has been found.
    std::unordered_map<Symbol, bool> finishedNT;

    using ProdSet = std::unordered_set<std::size_t>;
    std::function<void(std::size_t, ProdSet&)> updateNullability = [&](std::size_t index, ProdSet& visited) {
        const Production& prod = productions[index];
        if (visited.count(index) > 0|| finishedNT.count(prod.name) > 0) {
            return;
        }
        visited.insert(index);
        prod.firstSet.clear();
        prod.nullable = false;
        bool allNullable = true;
        // Tries to find the answer without recursion
        ECHO("[START] " + toBNF(prod));
        for (auto& symbol : prod.products) {
            TRACE_L("\tsymbol", symbol);
            if (isTerminal(symbol)) {
                ECHO("\t[NOT NULL]");
                return;
            }
            if (finishedNT.count(symbol) == 0) {
                allNullable = false;
            } else if (finishedNT[symbol] == !NULLABLE) {
                ECHO("\t[NOT NULL]");
                return;
            }
        }

        if (allNullable) {
            finishedNT[prod.name] = NULLABLE;
            prod.nullable = true;
            ECHO("\t[NULLABLE]");
            return;
        }

        // Recursion is necessary
        ECHO("\t[RECURSION MODE]");
        for (auto& symbol : prod.products) {
            if (finishedNT.count(symbol) > 0) {
                // symbol is a nullable non-terminal (the previous loop would
                // have returned if it wasn't).
                continue;
            }

            std::vector<std::size_t> indexList = productionsBySymbol.at(symbol);
            for (std::size_t i : indexList) {
                updateNullability(i, visited);
            }

            if (finishedNT.count(symbol) == 0) {
                // If no production marked the symbol as nullable,
                // then it isn't.
                finishedNT[symbol] = !NULLABLE;
                ECHO("\t[CONCLUSION] " + symbol + " is NOT null");
                return;
            }
        }

        // If we are here, the production is nullable.
        finishedNT[prod.name] = NULLABLE;
        prod.nullable = true;
        ECHO("\t[NULLABLE]");
    };

    // Calculates nullability
    ProdSet visited;
    for (std::size_t counter = 0; counter < size(); counter++) {
        updateNullability(counter, visited);
    }

    ECHO("#######################################");
    ECHO("#######################################");
    for (auto& pair : productionsBySymbol) {
        // bool nullable = false;
        // std::unordered_set<Symbol> result;
        // for (auto index : pair.second) {
        //     auto& prod = productions[index];
        //     if (prod.nullable) {
        //         nullable = true;
        //     }
        //     for (auto& symbol : prod.firstSet) {
        //         result.insert(symbol);
        //     }
        // }
        TRACE(pair.first);
        TRACE(finishedNT[pair.first]);
        // TRACE_IT(result);
        ECHO("-----");
    }

    isFirstValid = true;
    assert(false);
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
