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
        if (isTerminal(symbol) || !nullabilityBySymbol[symbol]) {
            return false;
        }
    }
    return true;
}

std::unordered_set<CFG::Symbol> CFG::range(const CFG::BNF& symbolSequence) const {
    std::vector<Symbol> symbols = toSymbolSequence(symbolSequence);
    std::unordered_set<Symbol> result;
    for (auto& symbol : symbols) {
        std::unordered_set<std::size_t> visited;
        if (populateRangeBySymbol(symbol, result, visited, false)) {
            break;
        }
    }
    return result;
}

bool CFG::isRecursive() const {
    auto nonTerminals = getNonTerminals();
    for (auto& symbol : nonTerminals) {
        if (range(symbol).count(symbol) > 0) {
            return true;
        }
    }
    return false;
}

CFG::ReferenceType CFG::recursionType(const CFG::Symbol& symbol) const {
    if (isTerminal(symbol)) {
        return ReferenceType::NONE;
    }

    for (std::size_t index : productionsBySymbol.at(symbol)) {
        const Production& prod = productions[index];
        for (auto& s : prod.products) {
            if (s == symbol) {
                return ReferenceType::DIRECT;
            }

            if (isTerminal(s) || !nullable(s)) {
                break;
            }
        }
    }

    if (range(symbol).count(symbol) > 0) {
        return ReferenceType::INDIRECT;
    }

    return ReferenceType::NONE;
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

    // Stores information about all non-terminals for which a definitive
    // conclusion about nullability has been found.
    nullabilityBySymbol.clear();
    std::unordered_map<Symbol, bool>& nullabilityTable = nullabilityBySymbol;

    // Avoids infinite loops in recursive grammars
    using ProdSet = std::unordered_set<std::size_t>;
    ProdSet visited;

    // Calculates nullability of all non-terminals
    for (std::size_t counter = 0; counter < size(); counter++) {
        updateNullability(counter, visited, nullabilityTable);
    }

    std::unordered_map<Symbol, std::unordered_set<Symbol>> firstTable;

    auto push = [&firstTable](const Production& production, const Symbol& symbol) {
        production.firstSet.insert(symbol);
        firstTable[production.name].insert(symbol);
    };

    std::function<void(std::size_t, ProdSet&)> populate = [&](std::size_t index, ProdSet& visited) {
        const Production& prod = productions[index];
        if (visited.count(index) > 0) {
            return;
        }
        visited.insert(index);
        // ECHO(toBNF(prod));
        for (auto& symbol : prod.products) {
            if (isTerminal(symbol)) {
                // ECHOI(symbol + " -> FIRST(" + toBNF(prod) + ")", 1);
                push(prod, symbol);
                return;
            }

            std::vector<std::size_t> indexList = productionsBySymbol.at(symbol);
            // ECHO("[RECURSION]");
            for (std::size_t i : indexList) {
                // TRACE(i);
                populate(i, visited);
            }
            // ECHO("----");

            for (auto& s : firstTable[symbol]) {
                // ECHOI(s + " -> FIRST(" + toBNF(prod) + ")", 1);
                push(prod, s);
            }

            if (!nullabilityTable[symbol]) {
                return;
            }
        }

        prod.nullable = true;
    };

    // The first iteration calculates the preliminary first set of all
    // productions; the second ensures all incomplete first sets are fixed.
    for (unsigned iter = 0; iter < 2; iter++) {
        visited.clear();
        for (std::size_t counter = 0; counter < size(); counter++) {
            populate(counter, visited);
        }
        // ECHO("#############################################");
    }

    // ECHO("#######################################");
    // ECHO("#######################################");
    // for (auto& prod : productions) {
    //     ECHO(toBNF(prod));
    //     TRACE(prod.nullable);
    //     TRACE_IT(prod.firstSet);
    //     ECHO("");
    // }
    // for (auto& pair : productionsBySymbol) {
    //     std::unordered_set<Symbol> result;
    //     for (auto index : pair.second) {
    //         auto& prod = productions[index];
    //         for (auto& symbol : prod.firstSet) {
    //             result.insert(symbol);
    //         }
    //     }
    //     TRACE(pair.first);
    //     TRACE(nullabilityTable[pair.first]);
    //     TRACE_IT(result);
    //     ECHO("-----");
    // }

    isFirstValid = true;
    // assert(false);
}

void CFG::updateNullability(std::size_t index, std::unordered_set<std::size_t>& visited,
    std::unordered_map<Symbol, bool>& finishedNT) const {

    // Just to improve readability.
    const bool NULLABLE = true;

    const Production& prod = productions[index];
    if (visited.count(index) > 0 || finishedNT.count(prod.name) > 0) {
        return;
    }
    visited.insert(index);
    prod.firstSet.clear();
    prod.nullable = false;
    bool allNullable = true;
    // Tries to find the answer without recursion
    // ECHO("[START] " + toBNF(prod));
    for (auto& symbol : prod.products) {
        // TRACE_L("\tsymbol", symbol);
        if (isTerminal(symbol)) {
            // ECHO("\t[NOT NULL]");
            return;
        }
        if (finishedNT.count(symbol) == 0) {
            allNullable = false;
        } else if (finishedNT[symbol] == !NULLABLE) {
            // ECHO("\t[NOT NULL]");
            return;
        }
    }

    if (allNullable) {
        finishedNT[prod.name] = NULLABLE;
        prod.nullable = true;
        // ECHO("\t[NULLABLE]");
        return;
    }

    // Recursion is necessary
    // ECHO("\t[RECURSION MODE]");
    for (auto& symbol : prod.products) {
        if (finishedNT.count(symbol) > 0) {
            // symbol is a nullable non-terminal (the previous loop would
            // have returned if it wasn't).
            continue;
        }

        std::vector<std::size_t> indexList = productionsBySymbol.at(symbol);
        for (std::size_t i : indexList) {
            updateNullability(i, visited, finishedNT);
        }

        if (finishedNT.count(symbol) == 0) {
            // If no production marked the symbol as nullable,
            // then it isn't.
            finishedNT[symbol] = !NULLABLE;
            // ECHO("\t[CONCLUSION] " + symbol + " is NOT null");
            return;
        }
    }

    // If we are here, the production is nullable.
    finishedNT[prod.name] = NULLABLE;
    prod.nullable = true;
    // ECHO("\t[NULLABLE]");
}

void CFG::populateRange(std::size_t index, std::unordered_set<CFG::Symbol>& result,
    std::unordered_set<std::size_t>& visited) const {

    if (visited.count(index) > 0) {
        return;
    }
    visited.insert(index);
    const Production& prod = productions[index];
    for (auto& symbol : prod.products) {
        if (populateRangeBySymbol(symbol, result, visited)) {
            return;
        }
    }
}

bool CFG::populateRangeBySymbol(const Symbol& symbol, std::unordered_set<CFG::Symbol>& result,
    std::unordered_set<std::size_t>& visited, bool push) const {

    if (isTerminal(symbol)) {
        return true;
    }

    if (push) {
        result.insert(symbol);
    }

    for (std::size_t index : productionsBySymbol.at(symbol)) {
        populateRange(index, result, visited);
    }

    return !nullable(symbol);
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
