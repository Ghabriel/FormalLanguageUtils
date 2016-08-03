/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include <stack>
#include "Lexer.hpp"
#include "utils.hpp"

void Lexer::ignore(char c) {
    blacklist.insert(c);
}

void Lexer::addToken(const TokenType& tokenType, const Expression& expr) {
    tokenTypes.insert(std::make_pair(tokenType, Regex(expr)));
}

void Lexer::removeToken(const TokenType& tokenType) {
    tokenTypes.erase(tokenType);
}

bool Lexer::accepts() const {
    return errorMessage.empty();
}

const std::string& Lexer::getError() const {
    return errorMessage;
}

std::vector<Token> Lexer::read(const std::string& input) {
    std::vector<Token> tokens;
    std::size_t i = 0;
    std::size_t length = input.size();
    while (i < length) {
        std::pair<std::size_t, Token> pair;
        try {
            pair = readNext(i, input);
        } catch (std::string err) {
            errorMessage = err;
            return tokens;
        }
        tokens.push_back(std::move(pair.second));
        i = pair.first;
    }
    return tokens;
}

std::pair<std::size_t, Token> Lexer::readNext(std::size_t startingIndex,
    const std::string& input) {

    std::unordered_set<TokenType> notAborted;
    for (auto& pair : tokenTypes) {
        notAborted.insert(pair.first);
        pair.second.reset();
    }

    std::unordered_map<TokenType, std::size_t> stateHistory;

    std::size_t i = startingIndex;
    std::size_t length = input.size();
    auto pick = [&]() {
        bool matched = false;
        TokenType type;
        std::size_t maxIndex;
        for (auto& pair : stateHistory) {
            if (!matched || pair.second > maxIndex) {
                matched = true;
                type = pair.first;
                maxIndex = pair.second;
            }
        }

        if (!matched) {
            maxIndex = i;
        }

        std::string buffer;
        for (i = startingIndex; i <= maxIndex; i++) {
            if (blacklist.count(input[i]) == 0) {
                buffer += input[i];
            }
        }

        if (!matched) {
            throw "Unknown symbol '" + buffer + "'";
        }

        return std::make_pair(maxIndex + 1, Token{type, buffer});
    };

    bool foundRelevantSymbol = false;
    while (i < length) {
        char c = input[i];
        if (blacklist.count(c) > 0) {
            if (foundRelevantSymbol) {
                return pick();
            }
            i++;
            continue;
        }

        foundRelevantSymbol = true;
        auto it = notAborted.begin();
        while (it != notAborted.end()) {
            const TokenType& type = *it;
            Regex& recognizer = tokenTypes.at(type);
            recognizer.read(c);
            if (recognizer.matches()) {
                stateHistory[type] = i;
            }

            if (recognizer.aborted()) {
                it = notAborted.erase(it);
            } else {
                it++;
            }
        }

        if (notAborted.size() == 0) {
            return pick();
        }
        i++;
    }
    return pick();
}
