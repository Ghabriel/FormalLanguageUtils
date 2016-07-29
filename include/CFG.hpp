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
 *   'p': maximum number of symbols on the right-hand side of the productions;
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

    enum ReferenceType {
        NONE,
        DIRECT,
        INDIRECT
    };

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
    // Complexity: O(s + L) on first call, O(L) on subsequent calls
    std::unordered_set<Symbol> first(const BNF&) const;

    // Returns the follow set of a given non-terminal.
    // The first call is slower due to multiple follow set calculations,
    // but subsequent calls are faster due to caching, until invalidated.
    // Returns an empty set for terminals.
    // Complexity:
    //     O(np^2) on first call if it's a non-terminal,
    //     O(1) on subsequent calls or if it's a terminal
    std::unordered_set<Symbol> follow(const Symbol&) const;

    // Checks if a sequence of symbols is able to derive the empty string.
    // Complexity: O(s + L) on first call, O(L) on subsequent calls
    bool nullable(const BNF&) const;

    // Checks if a given non-terminal is endable, i.e, can be the
    // last symbol of a derivation sequence. Returns false for terminals.
    // Complexity:
    //     O(np^2) on first call if it's a non-terminal,
    //     O(1) on subsequent calls or if it's a terminal
    bool endable(const Symbol&) const;

    // Returns the set of non-terminals that are left-reachable
    // by a given sequence of symbols.
    // Complexity: O(sL)
    std::unordered_set<Symbol> range(const BNF&) const;

    // Checks if this CFG is left-recursive.
    // Complexity: O(ns)
    bool isRecursive() const;

    // Returns the recursion type of a non-terminal.
    // Complexity: O(sL)
    ReferenceType recursionType(const Symbol&) const;

    // Checks if this CFG is factored.
    // Complexity: O(n(n^2 + s)) on first call, O(n^3) on subsequent calls
    bool isFactored() const;

    // Returns the non-factoring type of a non-terminal.
    // Complexity: O(n^2 + s) on first call, O(n^2) on subsequent calls
    ReferenceType nonFactoringType(const Symbol&) const;

    CFG withoutRecursion() const;

    // Checks if two CFGs are equal. Note that, since CFG equivalence
    // is undecidable, this implementation considers two CFGs equal
    // if and only if they have exactly the same productions.
    // Complexity: O(s1 + s2)
    bool operator==(const CFG&) const;

    // Returns a BNF representation of this CFG.
    // Complexity: O(s)
    BNF debug() const;

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
        mutable bool nullable;
    };
    std::vector<Production> productions;
    std::unordered_map<Symbol, std::vector<std::size_t>> productionsBySymbol;
    std::unordered_set<Symbol> nonTerminals;
    std::unordered_set<Symbol> terminals;
    mutable bool isFirstValid = false;
    mutable bool isFollowValid = false;
    mutable std::unordered_map<Symbol, bool> nullabilityBySymbol;
    mutable std::unordered_map<Symbol, std::unordered_set<Symbol>> followSet;
    mutable std::unordered_set<Symbol> endableNonTerminals;

    // Converts a BNF string to a sequence of symbols.
    // Complexity: O(L)
    std::vector<Symbol> toSymbolSequence(const BNF&) const;

    // Checks if a symbol corresponds to a terminal.
    // Complexity: O(1)
    bool isTerminal(const Symbol&) const;

    // Executes a callback for all productions of a given symbol.
    // Complexity: O(f), where f is the complexity of the callback.
    void select(const Symbol&, const std::function<void(const Production&)>&) const;

    // Calculates the first set of all non-terminals of this CFG.
    // Complexity: O(s) on first call, O(1) on subsequent calls
    void updateFirst() const;

    // Updates the nullability information about a production
    // and all other productions it references.
    // Complexity: O(s)
    void updateNullability(std::size_t, std::unordered_set<std::size_t>&, std::unordered_map<Symbol, bool>&) const;

    // Updates the range information about a production and all
    // other productions it references.
    // Complexity: O(s)
    void populateRange(std::size_t, std::unordered_set<Symbol>&, std::unordered_set<std::size_t>&) const;

    // Updates the range information about a symbol and all
    // other symbols it references.
    // Complexity: O(s)
    bool populateRangeBySymbol(const Symbol&, std::unordered_set<Symbol>&,
        std::unordered_set<std::size_t>&, bool = true) const;

    // Invalidates all cached structures.
    // Complexity: O(1)
    void invalidate();

    // Returns a BNF representation of a production.
    // Complexity: O(p)
    std::string toBNF(const Production&) const;

    // Returns the name of a symbol.
    // Complexity: O(1) if it's a terminal, otherwise O(length of name)
    std::string name(const Symbol&) const;
};

template<typename... Args>
CFG& CFG::add(const Symbol& name, const BNF& production, Args... args) {
    add(name, production);
    return add(name, args...);
}

#endif
