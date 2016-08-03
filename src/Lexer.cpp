/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include <stack>
#include <utility>
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
    errorMessage.clear();
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

        if (pair.second.type != "") {
            tokens.push_back(std::move(pair.second));
        }
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
    bool foundRelevantSymbol = false;
    auto pick = [&]() {
        if (!foundRelevantSymbol) {
            return std::make_pair(length, Token{"", ""});
        }

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
            throw error(input, startingIndex, i);
        }

        std::string buffer;
        for (i = startingIndex; i <= maxIndex; i++) {
            if (blacklist.count(input[i]) == 0) {
                buffer += input[i];
            }
        }

        return std::make_pair(maxIndex + 1, Token{type, buffer});
    };
    while (i < length) {
        char c = input[i];
        if (foundRelevantSymbol) {
            for (auto& regex : delimiters) {
                if (regex.matches(c)) {
                    return pick();
                }
            }
        }

        if (blacklist.count(c) > 0) {
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
            throw error(input, startingIndex, i);
        }
        i++;
    }
    return pick();
}

void Lexer::addDelimiters(const std::initializer_list<char>& list) {
    for (char c : list) {
        delimiters.push_back(Regex("\\" + std::string(1, c)));
    }
}

void Lexer::addDelimiters(const std::string& expr) {
    delimiters.push_back(Regex(expr));
}

std::string Lexer::error(const std::string& input, std::size_t from,
    std::size_t to) const {

    std::string buffer;
    for (std::size_t i = from; i <= to; i++) {
        if (blacklist.count(input[i]) == 0) {
            buffer += input[i];
        }
    }
    return "Unknown symbol '" + buffer + "'";
}
