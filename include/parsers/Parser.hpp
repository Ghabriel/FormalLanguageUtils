/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#ifndef PARSER_HPP
#define PARSER_HPP

#include <queue>
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
    enum class Action {
        ACCEPT,
        GOTO,
        REDUCE,
        SHIFT,
        UNKNOWN
    };

    struct LR0Item {
        std::size_t productionNumber;
        std::size_t position;
        mutable Action action = Action::UNKNOWN;
        mutable std::size_t targetState;
    };

    inline bool operator==(const LR0Item& lhs, const LR0Item& rhs) {
        return lhs.productionNumber == rhs.productionNumber && lhs.position == rhs.position;
    }
}

namespace std {
    template<>
    struct hash<parser::LR0Item> {
        std::size_t operator()(const parser::LR0Item& item) const {
            return item.productionNumber + item.position * 1e5;
        }
    };
}

namespace parser {
    struct LR0State {
        std::unordered_set<LR0Item> kernel;
        std::vector<LR0Item> items;
    };

    inline void expandState(LR0State& state, const CFG& cfg) {
        for (auto& item : state.kernel) {
            const Production& prod = cfg[item.productionNumber];
            if (item.position >= prod.size()) {
                continue;
            }

            auto symbol = prod[item.position];
            if (cfg.isNonTerminal(symbol)) {
                for (std::size_t i = 0; i < cfg.size(); i++) {
                    const Production& production = cfg[i];
                    if (production.getName() == symbol) {
                        state.items.emplace_back(LR0Item{i, 0});
                    }
                }
            }
        }
    }

    inline void printState(const LR0State& state, const CFG& cfg) {
        ECHO("Kernel:");
        for (auto& item : state.kernel) {
            ECHO(cfg.toReadableForm(cfg[item.productionNumber]));
            TRACE(item.position);
            TRACE(item.targetState);
        }
        ECHO("Items:");
        for (auto& item : state.items) {
            ECHO(cfg.toReadableForm(cfg[item.productionNumber]));
            TRACE(item.position);
            TRACE(item.targetState);
        }
        // TRACE(state.kernel.size());
        // TRACE(state.items.size());
        ECHO("");
    }

    // Returns the LR(0) Collection of a CFG.
    inline std::vector<LR0State> LR0(const CFG& cfg) {
        auto copy = cfg;
        std::size_t last = copy.size();
        copy << "<S'> ::= " + cfg[0].getName() + "'EOS'";

        std::vector<LR0State> result;
        result.emplace_back();
        result.back().kernel.emplace(LR0Item{last, 0});

        std::queue<std::size_t> pendingStates;
        pendingStates.push(0);

        while (!pendingStates.empty()) {
            LR0State& state = result[pendingStates.front()];
            pendingStates.pop();

            expandState(state, copy);
            std::unordered_map<Parser::Symbol, std::unordered_set<LR0Item>> itemsPerTransition;
            for (LR0Item& item : state.items) {
                const Production& prod = copy[item.productionNumber];
                if (item.position >= prod.size()) {
                    continue;
                }

                if (item.productionNumber == last && item.position == 1) {
                    item.action = Action::ACCEPT;
                    continue;
                }

                LR0Item newItem{item.productionNumber, item.position + 1};
                itemsPerTransition[prod[item.position]].insert(std::move(newItem));
            }

            for (auto& pair : itemsPerTransition) {
                auto& kernel = pair.second;
                std::size_t targetState;
                bool foundEqual = false;
                for (std::size_t i = 0; i < result.size(); i++) {
                    LR0State& state = result[i];
                    if (state.kernel == kernel) {
                        targetState = i;
                        foundEqual = true;
                        break;
                    }
                }

                if (!foundEqual) {
                    targetState = result.size();
                    result.emplace_back();
                    result.back().kernel = kernel;
                    pendingStates.push(targetState);
                }

                for (auto& item : kernel) {
                    for (auto& i : state.items) {
                        if (i.productionNumber == item.productionNumber
                            && i.position + 1 == item.position) {

                            i.action = Action::SHIFT;
                            i.targetState = targetState;
                        }
                    }
                    item.action = Action::SHIFT;
                    item.targetState = targetState;
                }
            }
        }

        for (auto& state : result) {
            printState(state, copy);
        }
        ECHO("END");
        return result;
    }
}

#endif
