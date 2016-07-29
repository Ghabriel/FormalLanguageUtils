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