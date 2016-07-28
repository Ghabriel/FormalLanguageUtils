#ifndef CFG_HPP
#define CFG_HPP

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "utils.hpp"

/*
 * A class that represents a Context-Free Grammar.
 * In the complexity comments, the following definitions apply:
 *   'n': number of productions of the CFG;
 *   's': number of symbols (both terminals and non-terminals) of the CFG;
 *   'L': number of symbols in the BNF string received as input.
 * Note that the result of several operations is cached to improve
 * performance. Some of these cached structures are invalidated when
 * a new production is added to the CFG, although it doesn't necessarily
 * happen.
 */
class CFG {
public:
    using Symbol = std::string;
    using BNF = std::string;

    // Adds production(s) to this CFG, returning itself to allow chaining.
    template<typename... Args>
    CFG& add(const Symbol&, const BNF&, Args...);
    CFG& add(const Symbol&, const BNF&);
    CFG& add(const BNF&);
    CFG& operator<<(const BNF&);

    // Clears this CFG.
    void clear();

    // Returns the number of productions in this CFG.
    // Complexity: O(1)
    std::size_t size() const;

    // Returns all non-terminals used in this CFG.
    // Complexity: O(1)
    std::unordered_set<Symbol> getNonTerminals() const;

    // Returns all terminals used in this CFG.
    // Complexity: O(1)
    std::unordered_set<Symbol> getTerminals() const;

    // Checks if there are any undefined non-terminals
    // being used in this CFG.
    // Complexity: O(s)
    bool isConsistent() const;

    // Returns the first set of a sequence of symbols.
    // The first call is slower due to multiple first set calculations,
    // but subsequent calls are faster due to caching, until invalidated.
    // Complexity:
    std::unordered_set<Symbol> first(const BNF&) const;

    // Checks if a sequence of symbols is able to derive the empty string.
    // Complexity:
    bool nullable(const BNF&) const;

private:
    class Production {
        friend class CFG;
    public:
        explicit Production(const std::string& name) : name(name) {}
        std::size_t size() const {
            return products.size();
        }
        const Symbol& operator[](std::size_t index) const {
            return products[index];
        }
        Symbol& operator[](std::size_t index) {
            return products[index];
        }
    private:
        std::string name;
        std::vector<Symbol> products;
        mutable std::unordered_set<Symbol> firstSet;
        // mutable std::unordered_set<Symbol> followSet;
        mutable bool nullable;
    };
    std::vector<Production> productions;
    std::unordered_map<Symbol, std::vector<std::size_t>> productionsBySymbol;
    std::unordered_set<Symbol> nonTerminals;
    std::unordered_set<Symbol> terminals;
    mutable bool isFirstValid = false;

    // Converts a BNF string to a sequence of symbols.
    // Complexity: O(L)
    std::vector<Symbol> toSymbolSequence(const BNF&) const;

    // Checks if a symbol corresponds to a terminal.
    // Complexity: O(1)
    bool isTerminal(const Symbol&) const;

    // Executes a callback for all productions of a given symbol.
    // Complexity: O(f), where f is the complexity of the callback.
    void select(const Symbol&, const std::function<void(const Production&)>&) const;

    void updateFirst() const;
    void updateNullability(std::size_t, std::unordered_set<std::size_t>&, std::unordered_map<Symbol, bool>&) const;

    void invalidate();

    std::string toBNF(const Production&) const;
};

template<typename... Args>
CFG& CFG::add(const Symbol& name, const BNF& production, Args... args) {
    add(name, production);
    return add(name, args...);
}

#endif
