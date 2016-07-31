/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#include <stack>
#include "Lexer.hpp"
#include "utils.hpp"

void Lexer::ignore(char c) {
    blacklist.insert(c);
}

void Lexer::addToken(const TokenType& tokenType, const Expression& expr) {
    tokenTypes.insert(std::make_pair(tokenType, std::regex(expr)));
}

void Lexer::removeToken(const TokenType& tokenType) {
    tokenTypes.erase(tokenType);
}

std::vector<Token> Lexer::read(const std::string& input) const {
    std::vector<Token> tokens;
    std::size_t i = 0;
    std::size_t length = input.size();
    while (i < length) {
        auto pair = readNext(i, input);
        tokens.push_back(std::move(pair.second));
        i = pair.first;
    }
    return tokens;
}

std::pair<std::size_t, Token> Lexer::readNext(std::size_t startingIndex,
    const std::string& input) const {

    std::unordered_set<TokenType> candidates;
    for (auto& pair : tokenTypes) {
        candidates.insert(pair.first);
    }

    std::stack<TokenType> discardPile;
    std::string buffer;
    std::size_t length = input.size();
    for (std::size_t i = startingIndex; i < length; i++) {
        char c = input[i];

        if (blacklist.count(c) > 0) {
            if (candidates.size() > 1) {
                ECHO("Warning: ambiguous input");
            }
            return std::make_pair(i + 1, Token{*candidates.begin(), buffer});
        }

        auto it = candidates.begin();
        while (it != candidates.end()) {
            auto& tokenType = *it;
            auto& expression = tokenTypes.at(tokenType);
            std::string enhancedBuffer = buffer + c;
            std::smatch matches;
            std::regex_match(enhancedBuffer, matches, expression);
            if (matches.size() == 0) {
                discardPile.push(tokenType);
                it = candidates.erase(it);
            } else {
                it++;
            }
        }

        if (candidates.size() == 0) {
            return std::make_pair(i, Token{discardPile.top(), buffer});
        }
        buffer += c;
    }

    if (candidates.size() > 1) {
        ECHO("Warning: ambiguous input");
    }
    return std::make_pair(length, Token{*candidates.begin(), buffer});
}
