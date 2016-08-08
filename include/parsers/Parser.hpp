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

protected:
    ParseResults error(const std::vector<Token>& input,
        std::size_t index, const std::string& message) const {

        ParseResults result;
        result.accepted = false;
        result.errorIndex = index;
        result.errorMessage = "Error: " + message + "\n";
        for (std::size_t i = 0; i < index; i++) {
            result.errorMessage += input[i].content;
        }

        if (index < input.size()) {
            result.errorMessage += ("\033[1;31m" + input[index].content + "\033[0m");
            for (std::size_t i = index + 1; i < input.size(); i++) {
                result.errorMessage += input[i].content;
            }
        }

        return result;
    }

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
        for (auto& item : state.items) {
            // TRACE(item.productionNumber);
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
            // TRACE(item.position);
            // TRACE(item.targetState);
        }
        ECHO("Items:");
        for (auto& item : state.items) {
            ECHO(cfg.toReadableForm(cfg[item.productionNumber]));
            TRACE(item.position);
            TRACE(item.targetState);
            TRACE(static_cast<int>(item.action));
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

        // Creates the initial state
        result.emplace_back();
        result.back().kernel.emplace(LR0Item{last, 0});
        result.back().items.push_back(*result.back().kernel.begin());

        // Queue of not-yet-analyzed states
        std::queue<std::size_t> pendingStates;
        pendingStates.push(0);

        while (!pendingStates.empty()) {
            // Prevents the result vector from reallocating in the middle of
            // this iteration (which would invalidate the state reference)
            if (result.capacity() == result.size()) {
                result.reserve(result.size() + 10);
            }

            // The current state being analyzed
            LR0State& state = result[pendingStates.front()];
            pendingStates.pop();

            expandState(state, copy);
            std::unordered_map<Parser::Symbol, std::unordered_set<LR0Item>> itemsPerTransition;
            std::queue<LR0Item*> completedItems;

            // Maps all possible next symbols to a group of LR0 Items
            for (LR0Item& item : state.items) {
                const Production& prod = copy[item.productionNumber];
                if (item.position >= prod.size()) {
                    item.action = Action::REDUCE;
                    continue;
                }

                // <S'> ::= <S>.$ should accept
                if (item.productionNumber == last && item.position == 1) {
                    item.action = Action::ACCEPT;
                    continue;
                }

                // Pushes a copy of the item, with its position added by 1.
                // These items will constitute the kernel of the target state.
                LR0Item newItem{item.productionNumber, item.position + 1};
                itemsPerTransition[prod[item.position]].insert(std::move(newItem));
            }

            // Uses the calculated map to find the target states of all items
            // which have a SHIFT or GOTO action.
            for (auto& pair : itemsPerTransition) {
                auto& kernel = pair.second;
                std::size_t targetState;
                bool foundEqual = false;
                // Checks if there's another state with the same kernel
                for (std::size_t i = 0; i < result.size(); i++) {
                    LR0State& st = result[i];
                    if (st.kernel == kernel) {
                        targetState = i;
                        foundEqual = true;
                        break;
                    }
                }

                if (!foundEqual) {
                    // If no matches were found, create a new state
                    // and mark it as not-yet-analyzed
                    targetState = result.size();
                    result.emplace_back();
                    result.back().kernel = kernel;
                    result.back().items = std::vector<LR0Item>(kernel.begin(), kernel.end());
                    pendingStates.push(targetState);
                }

                // Assigns the appropriate action type to each item
                for (auto& item : kernel) {
                    const Production& prod = copy[item.productionNumber];
                    Action action = copy.isTerminal(prod[item.position - 1])
                                  ? Action::SHIFT
                                  : Action::GOTO;
                    for (auto& i : state.items) {
                        if (i.productionNumber == item.productionNumber
                            && i.position + 1 == item.position) {

                            i.action = action;
                            i.targetState = targetState;
                        }
                    }
                    item.action = action;
                    item.targetState = targetState;
                }
            }
        }

        // ECHO("########################");
        // std::size_t i = 0;
        // for (auto& state : result) {
        //     ECHO(i++);
        //     printState(state, copy);
        //     ECHO("----------------------");
        // }
        // ECHO("END");
        return result;
    }
}

#endif
