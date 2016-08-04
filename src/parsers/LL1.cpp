/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include "Lexer.hpp"
#include "parsers/LL1.hpp"
#include "utils.hpp"

namespace {
    const std::string END_OF_SENTENCE = "EOS";
}

parser::LL1::LL1(const CFG& cfg) : Parser(cfg) {
    cfg.prepareFirst();
    for (std::size_t i = 0; i < cfg.size(); i++) {
        const Production& prod = cfg[i];
        std::string prodName = prod.getName();
        auto& row = table[prodName];
        for (auto& symbol : prod.getFirstSet()) {
            if (row.count(symbol) > 0) {
                conflict = true;
                return;
            }
            row[symbol] = i;
        }

        if (prod.isNullable()) {
            auto follow = cfg.follow(prodName);
            if (cfg.endable(prodName)) {
                follow.insert(END_OF_SENTENCE);
            }
            for (auto& symbol : follow) {
                if (row.count(symbol) > 0) {
                    conflict = true;
                    return;
                }
                row[symbol] = i;
            }
        }
    }

    // for (auto& pair : table) {
    //     TRACE(pair.first);
    //     for (auto& p : pair.second) {
    //         ECHO(p.first + " -> " + std::to_string(p.second));
    //     }
    // }
}

ParseResults parser::LL1::parse(const std::vector<Token>& input) {
    assert(canParse());
    ParseResults result;
    std::size_t length = input.size();
    std::stack<TokenType> stack;
    stack.push(END_OF_SENTENCE);
    stack.push(getCFG()[0].getName());
    for (std::size_t i = 0; i <= length; i++) {
        const TokenType& symbol = (i < length) ? input[i].type : END_OF_SENTENCE;
        result = unwind(stack, symbol);
        if (!result.accepted) {
            return error(input, i, result.errorMessage);
        }

        if (stack.top() == symbol) {
            stack.pop();
            // ECHO("[POP] " + symbol);
        } else {
            return error(input, i, "Unexpected token '" + symbol + "', expected '" + stack.top() + "'");
        }
    }

    if (!stack.empty()) {
        return error(input, input.size(), "Unexpected end-of-sentence, expected '" + stack.top() + "'");
    }

    result.accepted = true;
    return result;
}

bool parser::LL1::canParse() const {
    return !conflict;
}

ParseResults parser::LL1::unwind(std::stack<Symbol>& stack, const Symbol& input) {
    ParseResults result;
    Symbol& top = stack.top();
    if (getCFG().isTerminal(top) || top == END_OF_SENTENCE) {
        result.accepted = true;
        return result;
    }

    auto& row = table[top];
    if (row.count(input) > 0) {
        std::size_t index = row[input];
        const Production& prod = getCFG()[index];
        stack.pop();
        // ECHO("[POP] " + top);
        for (const Symbol& symbol : utils::make_reverse(prod.getProducts())) {
            stack.push(symbol);
            // ECHO("[PUSH] " + symbol);
        }
        return unwind(stack, input);
    }

    result.accepted = false;
    result.errorMessage = "Unexpected token '" + input + "'";
    return result;
}

ParseResults parser::LL1::error(const std::vector<Token>& input,
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
