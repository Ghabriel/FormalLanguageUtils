/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef CFG_HPP
#define CFG_HPP

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class CFGRepresentation;
class CFG;

class Production {
    friend class CFG;
public:
    using Symbol = std::string;
    explicit Production(const std::string& name) : name(name) {}
    std::string getName() const {
        return name;
    }

    const std::vector<Symbol>& getProducts() const {
        return products;
    }

    const std::unordered_set<Symbol>& getFirstSet() const {
        return firstSet;
    }

    bool isNullable() const {
        return nullable;
    }

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
 *
 * The Strategy Pattern is used to decouple the grammar representation
 * from the functionality provided by this class. A static factory method
 * is used to allow such custom representations.
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

    CFG();

    template<typename T>
    static CFG create(const T& = *defaultRepresentation);

    template<typename T>
    static CFG create(const std::shared_ptr<T>&);

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

    // Prepares the first set of all productions.
    // Complexity: O(s) on first call, O(1) on subsequent calls
    void prepareFirst() const;

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

    // Returns a production, given its index.
    // Complexity: O(1)
    const Production& operator[](std::size_t) const;

    // Utility methods that map to the representation scheme methods.
    bool isTerminal(const Symbol&) const;
    bool isNonTerminal(const Symbol&) const;
    std::string toReadableForm(const Production&) const;

    // Changes the default representation of CFGs.
    template<typename T>
    static void setDefaultRepresentation(const T&) {
        defaultRepresentation.reset(new T());
    }

    // Returns a BNF representation of this CFG.
    // Complexity: O(s)
    BNF debug() const;

private:
    std::vector<Production> productions;
    std::unordered_map<Symbol, std::vector<std::size_t>> productionsBySymbol;
    std::unordered_set<Symbol> nonTerminals;
    std::unordered_set<Symbol> terminals;
    mutable bool isFirstValid = false;
    mutable bool isFollowValid = false;
    mutable std::unordered_map<Symbol, bool> nullabilityBySymbol;
    mutable std::unordered_map<Symbol, std::unordered_set<Symbol>> followSet;
    mutable std::unordered_set<Symbol> endableNonTerminals;

    std::shared_ptr<const CFGRepresentation> representation;
    static std::shared_ptr<const CFGRepresentation> defaultRepresentation;

    const CFGRepresentation& getRepresentation() const;

    CFG& internalAdd(const Production&);

    // Utility methods that map to the representation scheme methods.
    std::vector<Symbol> toSymbolSequence(const BNF&) const;
    std::string name(const Symbol&) const;

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
};

template<typename T>
CFG CFG::create(const T&) {
    CFG instance;
    instance.representation.reset(new T());
    return instance;
}

template<typename T>
CFG CFG::create(const std::shared_ptr<T>& ptr) {
    CFG instance;
    instance.representation = ptr;
    return instance;
}

template<typename... Args>
CFG& CFG::add(const Symbol& name, const BNF& production, Args... args) {
    add(name, production);
    return add(name, args...);
}

#endif
