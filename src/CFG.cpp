/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

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
    invalidate();
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

std::unordered_set<CFG::Symbol> CFG::follow(const Symbol& nonTerminal) const {
    if (isTerminal(nonTerminal)) {
        return {};
    }

    if (isFollowValid) {
        return followSet[nonTerminal];
    }

    const std::string END_OF_STRING = "$";

    followSet.clear();
    followSet[productions[0].name].insert(END_OF_STRING);
    std::unordered_map<Symbol, std::unordered_set<Symbol>> dependencies;
    // Calculates all first-based follow sets, keeping track of dependencies
    for (auto& prod : productions) {
        const std::string& name = prod.name;
        std::size_t numProducts = prod.size();
        for (std::size_t i = 0; i < numProducts; i++) {
            const Symbol& symbol = prod[i];
            if (!isTerminal(symbol)) {
                bool isNullable = true;
                for (std::size_t j = i + 1; j < numProducts; j++) {
                    for (auto& s : first(prod[j])) {
                        followSet[symbol].insert(s);
                    }

                    if (!nullable(prod[j])) {
                        isNullable = false;
                        break;
                    }
                }
                if (isNullable && symbol != name) {
                    dependencies[symbol].insert(name);
                }
            }
        }
    }

    // Solves all dependencies
    bool stable = false;
    while (!stable) {
        stable = true;
        for (auto& pair : dependencies) {
            const Symbol& destination = pair.first;
            std::size_t prevSize = followSet[destination].size();
            for (const Symbol& origin : pair.second) {
                for (const Symbol& symbol : followSet[origin]) {
                    followSet[destination].insert(symbol);
                }
            }
            if (followSet[destination].size() != prevSize) {
                stable = false;
            }
        }
    }

    endableNonTerminals.clear();
    std::unordered_set<Symbol> nonTerminals = getNonTerminals();
    // Removes END_OF_STRING of all follow sets and tags those who have it
    // as endable non-terminals.
    for (auto& symbol : nonTerminals) {
        if (followSet[symbol].count(END_OF_STRING) > 0) {
            endableNonTerminals.insert(symbol);
            followSet[symbol].erase(END_OF_STRING);
        }
    }

    // Uncomment in case of emergency
    // for (auto& pair : followSet) {
    //     TRACE(pair.first);
    //     TRACE_IT(pair.second);
    //     ECHO("");
    //     TRACE_IT(dependencies[pair.first]);
    //     ECHO("###################################");
    // }
    // assert(false);

    isFollowValid = true;
    return followSet[nonTerminal];
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

bool CFG::endable(const Symbol& symbol) const {
    if (isTerminal(symbol)) {
        return false;
    }
    follow(symbol);
    return endableNonTerminals.count(symbol) > 0;
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

bool CFG::isFactored() const {
    auto nonTerminals = getNonTerminals();
    for (auto& symbol : nonTerminals) {
        if (nonFactoringType(symbol) != ReferenceType::NONE) {
            return false;
        }
    }
    return true;
}

CFG::ReferenceType CFG::nonFactoringType(const CFG::Symbol& symbol) const {
    if (isTerminal(symbol)) {
        return ReferenceType::NONE;
    }
    updateFirst();
    std::unordered_set<Symbol> history;
    std::unordered_set<Symbol> firstSets;
    bool indirect = false;
    for (std::size_t index : productionsBySymbol.at(symbol)) {
        const Production& prod = productions[index];
        if (prod.size() == 0) {
            continue;
        }

        if (isTerminal(prod[0])) {
            if (history.count(prod[0]) > 0) {
                return ReferenceType::DIRECT;
            }
            history.insert(prod[0]);
        }

        if (!indirect) {
            for (auto& s : prod.firstSet) {
                if (firstSets.count(s) > 0) {
                    indirect = true;
                    break;
                }
                firstSets.insert(s);
            }
        }
    }

    return indirect ? ReferenceType::INDIRECT : ReferenceType::NONE;
}

CFG CFG::withoutRecursion() const {
    CFG result;
    auto nonTerminals = getNonTerminals();
    // unsigned counter = 0;
    for (auto& nonTerminal : nonTerminals) {
        ReferenceType recType = recursionType(nonTerminal);
        if (recType == ReferenceType::NONE) {
            for (std::size_t index : productionsBySymbol.at(nonTerminal)) {
                const Production& prod = productions[index];
                result << toBNF(prod);
            }
            continue;
        }
        if (recType == ReferenceType::INDIRECT) {
            ECHO("NOT YET IMPLEMENTED");
            assert(false);
        }

        Symbol newNT = "<" + name(nonTerminal) + "'>";
        for (std::size_t index : productionsBySymbol.at(nonTerminal)) {
            const Production& prod = productions[index];
            std::size_t i;
            BNF newProd;
            if (prod.size() > 0 && prod[0] == nonTerminal) {
                i = 1;
                newProd = newNT;
            } else {
                i = 0;
                newProd = nonTerminal;
            }
            newProd += " ::= ";
            while (i < prod.size()) {
                newProd += prod[i];
                i++;
            }
            newProd += newNT;
            result << newProd;
        }
        result << newNT + " ::= ";
    }


    return result;
}

bool CFG::operator==(const CFG& other) const {
    if (size() != other.size()) {
        return false;
    }
    std::unordered_set<BNF> productionList;
    for (auto& production : productions) {
        productionList.insert(toBNF(production));
    }

    for (auto& production : other.productions) {
        if (productionList.count(toBNF(production)) == 0) {
            return false;
        }
    }
    return true;
}

CFG::BNF CFG::debug() const {
    BNF content;
    for (auto& symbol : getNonTerminals()) {
        content += symbol + " ::= ";
        bool ignore = true;
        for (std::size_t index : productionsBySymbol.at(symbol)) {
            const Production& prod = productions[index];
            if (!ignore) {
                content += "|";
            } else {
                ignore = false;
            }
            for (auto& s : prod.products) {
                content += s;
            }
        }
        content += '\n';
    }
    return content;
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
    isFollowValid = false;
}

std::string CFG::toBNF(const Production& prod) const {
    std::string result = prod.name + " ::= ";
    for (auto& symbol : prod.products) {
        result += symbol;
    }
    return result;
}

std::string CFG::name(const CFG::Symbol& symbol) const {
    if (isTerminal(symbol)) {
        return symbol;
    }
    return symbol.substr(1, symbol.size() - 2);
}
