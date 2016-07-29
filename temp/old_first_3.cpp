void CFG::updateFirst() const {
    if (isFirstValid) {
        return;
    }
    std::unordered_map<Symbol, std::unordered_set<Symbol>> result;
    std::unordered_set<Symbol> nullableList;
    std::unordered_set<Symbol> nonTerminals = getNonTerminals();
    std::unordered_set<Symbol> visited;
    std::queue<Symbol> queue;

    std::function<void(const Symbol&)> populate = [&](const Symbol& name) {
        if (visited.count(name) > 0) {
            return;
        }
        visited.insert(name);
        for (auto& index : productionsBySymbol.at(name)) {
            const Production& prod = productions[index];
            std::size_t j = 0;
            std::size_t numProducts = prod.size();
            while (j < numProducts) {
                const Symbol& symbol = prod[j];
                if (isTerminal(symbol)) {
                    result[name].insert(symbol);
                    break;
                }
                populate(symbol);
                for (auto& s : result[symbol]) {
                    result[name].insert(s);
                }
                if (nullableList.count(symbol) == 0) {
                    break;
                }
                j++;
            }
            if (j == numProducts) {
                nullableList.insert(name);
                queue.push(name);
            }
        }
    };

    for (Symbol name : nonTerminals) {
        queue.push(name);
    }

    unsigned counter = 0;
    while (!queue.empty()) {
        if (counter < nonTerminals.size()) {
            visited.clear();        
        }
        counter++;
        auto& name = queue.front();
        queue.pop();
        populate(name);
    }

    isFirstValid = true;

    // ECHO("####################");
    // for (auto& pair : result) {
    //     TRACE(pair.first);
    //     TRACE(nullableList.count(pair.first) > 0);
    //     TRACE_IT(pair.second);
    //     ECHO("-----");
    // }

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
}