/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <unordered_set>
#include <vector>
#include "CFG.hpp"
#include "utils.hpp"

struct Token;

struct ParseResults {
    bool accepted;
    std::size_t errorIndex;
    std::string errorMessage;
};

struct LR0Item {
    std::size_t productionNumber;
    std::size_t position;
};

namespace std {
    template<>
    struct hash<LR0Item> {
        std::size_t operator()(const LR0Item& item) const {
            return item.productionNumber + item.position * 1e5;
        }
    };
}

struct LR0State {
    std::unordered_set<LR0Item> kernel;
    std::unordered_set<LR0Item> items;
};

class Parser {
public:
    using Symbol = std::string;
    using TokenType = std::string;

    Parser(const CFG& cfg) : cfg(cfg) {}
    const CFG& getCFG() const {
        return cfg;
    }

    virtual ParseResults parse(const std::vector<Token>&) = 0;
    virtual bool canParse() const = 0;

private:
    CFG cfg;
};

namespace parser {
    // Returns the LR(0) Collection of a CFG.
    inline std::vector<LR0State> LR0(const CFG& cfg) {
        std::vector<LR0State> result;
        ECHO("NOT YET IMPLEMENTED");
        assert(false);
        return result;
    }
}

#endif
