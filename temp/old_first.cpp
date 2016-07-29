void CFG::updateFirst() const {
    if (isFirstValid) {
        return;
    }
    std::unordered_map<Symbol, std::unordered_set<Symbol>> registryByNonTerminal;
    std::vector<std::unordered_set<Symbol>> registry;
    std::unordered_map<Symbol, bool> nullability;
    std::unordered_map<Symbol, int> uncertainNonTerminals;
    std::queue<std::size_t> uncertain;
    registryByNonTerminal.reserve(size());
    registry.resize(size());
    nullability.reserve(size());
    uncertainNonTerminals.reserve(size());

    // Preliminary first check, filtering out all trivial firsts
    for (std::size_t i = 0; i < size(); i++) {
        const Production& prod = productions[i];
        if (nullability.count(prod.name) == 0) {
            nullability[prod.name] = false;
        }
        if (uncertainNonTerminals.count(prod.name) == 0) {
            uncertainNonTerminals[prod.name] = 0;
        }
        if (prod.products.size() == 0) {
            nullability[prod.name] = true;
        } else {
            const Symbol& startingSymbol = prod.products[0];
            if (isTerminal(startingSymbol)) {
                registryByNonTerminal[prod.name].insert(startingSymbol);
                registry[i].insert(startingSymbol);
            } else {
                uncertainNonTerminals[prod.name]++;
                uncertain.push(i);
            }
        }
    }

    while (!uncertain.empty()) {
        std::size_t index = uncertain.front();
        uncertain.pop();
        bool stillUncertain = false;
        bool isNullable = true;
        const Production& prod = productions[index];
        for (auto& symbol : prod.products) {
            if (isTerminal(symbol)) {
                registryByNonTerminal[prod.name].insert(symbol);
                registry[index].insert(symbol);
                isNullable = false;
                break;
            }
            if (uncertainNonTerminals[symbol] > 0) {
                uncertain.push(index);
                stillUncertain = true;
            } else if (symbol != prod.name) {
                for (auto& s : registryByNonTerminal[symbol]) {
                    registryByNonTerminal[prod.name].insert(s);
                    registry[index].insert(s);
                }
            }
            if (!nullability[symbol]) {
                isNullable = false;
                break;
            }
        }

        if (!stillUncertain) {
            uncertainNonTerminals[prod.name]--;
            if (isNullable) {
                nullability[prod.name] = true;
            }
        }
    }

    for (std::size_t i = 0; i < size(); i++) {
        productions[i].firstSet = registry[i];
        productions[i].nullable = ;
    }

    // for (auto& pair : registryByNonTerminal) {
    //     TRACE(pair.first);
    //     TRACE(nullability[pair.first]);
    //     TRACE_IT(pair.second);
    //     ECHO("-----");
    // }

    // ECHO("#######");
    // for (auto& pair : uncertainNonTerminals) {
    //     TRACE(pair.first);
    //     TRACE(pair.second);
    // }

    // for (auto& v : registry) {
    //     TRACE_IT(v);
    //     ECHO("-----");
    // }

    // for (unsigned i = 0; i < size(); i++) {
    //     TRACE(i);
    //     TRACE(productions[i].name);
    //     TRACE(nullability[productions[i].name]);
    //     TRACE_IT(registry[i]);
    //     ECHO("-----");
    // }
}