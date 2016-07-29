/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef CFGREPRESENTATION_HPP
#define CFGREPRESENTATION_HPP

#include <cassert>
#include <string>
#include <vector>

struct ProductionParts {
    std::string name;
    std::vector<std::string> products;
};

class CFGRepresentation {
public:
    virtual bool isTerminal(const std::string&) const = 0;
    virtual bool isNonTerminal(const std::string&) const = 0;
    virtual std::vector<ProductionParts> decompose(const std::string&) const = 0;
    virtual std::vector<std::string> toSymbolSequence(const std::string&) const = 0;
    virtual std::string toReadableForm(const std::string& name,
        const std::vector<std::string>& products) const = 0;
    virtual std::string name(const std::string&) const = 0;

    ProductionParts decompose(const std::string& name, const std::string& rhs) const {
        assert(isNonTerminal(name));
        ProductionParts production;
        production.name = name;
        production.products = toSymbolSequence(rhs);
        return production;
    }
};

#endif