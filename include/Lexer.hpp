/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Regex.hpp"

struct Token {
	std::string type;
	std::string content;
};

inline bool operator==(const Token& lhs, const Token& rhs) {
	return lhs.type == rhs.type && lhs.content == rhs.content;
}

inline std::ostream& operator<<(std::ostream& stream, const Token& token) {
	return stream << "Token(" + token.type + ", " + token.content + ")";
}

class Lexer {
public:
	using TokenType = std::string;
	using Expression = std::string;

	void ignore(char);
	void addToken(const TokenType&, const Expression&);
	void removeToken(const TokenType&);
	bool accepts() const;
	const std::string& getError() const;
	std::vector<Token> read(const std::string&);

private:
	std::unordered_map<TokenType, Regex> tokenTypes;
	std::unordered_set<char> blacklist;
	std::string errorMessage;

	std::pair<std::size_t, Token> readNext(std::size_t, const std::string&);
};

#endif
