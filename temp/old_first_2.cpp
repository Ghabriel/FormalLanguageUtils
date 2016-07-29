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
        ECHO(toBNF(prod));
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