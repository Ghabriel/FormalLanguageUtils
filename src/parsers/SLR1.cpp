/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include <stack>
#include "Lexer.hpp"
#include "parsers/SLR1.hpp"

parser::SLR1::SLR1(const CFG& cfg) : Parser(cfg) {
    auto lr0 = parser::LR0(cfg);
    auto copy = cfg;
    copy << "<S'> ::= " + cfg[0].getName() + "'EOS'";
    for (std::size_t i = 0; i < lr0.size(); i++) {
        LR0State& state = lr0[i];
        // TRACE(i);
        // parser::printState(state, copy);
        for (LR0Item& item : state.items) {
            auto& symbol = copy[item.productionNumber][item.position];
            const Production& prod = copy[item.productionNumber];
            switch (item.action) {
                case Action::ACCEPT:
                    if (table[i].count("EOS") > 0) {
                        ECHO("[CONFLICT] ACCEPT");
                        conflict = true;
                    }
                    table[i]["EOS"] = AscendingAction{item.action, 0};
                    break;
                case Action::GOTO:
                case Action::SHIFT:
                    if (table[i].count(symbol) > 0 && table[i][symbol].action != item.action) {
                        ECHO("[CONFLICT] S " + std::to_string(i) + " " + symbol);
                        conflict = true;
                    }
                    table[i][symbol] = AscendingAction{item.action, item.targetState};
                    break;
                case Action::REDUCE: {
                    auto follow = copy.follow(prod.getName());
                    for (auto& s : follow) {
                        if (table[i].count(s) > 0) {
                            ECHO("[CONFLICT] R " + std::to_string(i) + " " + s);
                            conflict = true;
                        }
                        table[i][s] = AscendingAction{item.action, item.productionNumber};
                    }
                    break;
                }
                default:
                    ECHO("????");
                    assert(false);
            }
        }
    }

    // for (auto& pair : table) {
    //     TRACE(pair.first);
    //     for (auto& p : pair.second) {
    //         std::string content;
    //         switch (p.second.action) {
    //             case Action::ACCEPT:
    //                 content = "ACCEPT";
    //                 break;
    //             case Action::GOTO:
    //                 content = std::to_string(p.second.target);
    //                 break;
    //             case Action::REDUCE:
    //                 content = "R" + std::to_string(p.second.target);
    //                 break;
    //             case Action::SHIFT:
    //                 content = "S" + std::to_string(p.second.target);
    //                 break;
    //             default:
    //                 ECHO("????");
    //                 assert(false);
    //         }
    //         ECHO(p.first + " -> " + content);
    //     }
    // }
}

ParseResults parser::SLR1::parse(const std::vector<Token>& tokens) {
    assert(canParse());
    ParseResults results;
    std::stack<std::size_t> stateStack;
    stateStack.push(0);
    std::size_t inputPointer = 0;
    Symbol nonTerminalBuffer;
    while (true) {
        TokenType currToken;
        if (!nonTerminalBuffer.empty()) {
            currToken = nonTerminalBuffer;
        } else if (inputPointer < tokens.size()) {
            currToken = tokens[inputPointer].type;
        } else {
            currToken = "EOS";
        }
        auto& currState = table[stateStack.top()];
        if (currState.count(currToken) == 0) {
            return error(tokens, inputPointer, "Unexpected token '" + currToken + "'");
        }

        AscendingAction& action = currState[currToken];
        switch (action.action) {
            case Action::ACCEPT:
                results.accepted = true;
                return results;
            case Action::GOTO:
                stateStack.push(action.target);
                nonTerminalBuffer.clear();
                break;
            case Action::REDUCE: {
                const Production& prod = getCFG()[action.target];
                for (std::size_t i = 0; i < prod.size(); i++) {
                    stateStack.pop();
                }
                nonTerminalBuffer = prod.getName();
                break;
            }
            case Action::SHIFT:
                stateStack.push(action.target);
                inputPointer++;
                break;
            default:
                assert(false);
        }
    }
}

bool parser::SLR1::canParse() const {
    return !conflict;
}
 