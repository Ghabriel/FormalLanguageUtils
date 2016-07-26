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
 *   's': number of symbols (both terminals and non-terminals) of the CFG.
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

private:
    class Production {
        friend class CFG;
    public:
        explicit Production(const std::string& name) : name(name) {}
    private:
        std::string name;
        std::vector<Symbol> products;
    };
    std::vector<Production> productions;
    std::unordered_set<Symbol> nonTerminals;
    std::unordered_set<Symbol> terminals;
};

template<typename... Args>
CFG& CFG::add(const Symbol& name, const BNF& production, Args... args) {
    add(name, production);
    return add(name, args...);
}

#endif