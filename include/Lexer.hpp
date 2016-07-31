/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <ostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Token {
	std::string type;
	std::string content;
};

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
	std::vector<Token> read(const std::string&) const;

private:
	std::unordered_map<TokenType, std::regex> tokenTypes;
	std::unordered_set<char> blacklist;

	std::pair<std::size_t, Token> readNext(std::size_t, const std::string&) const;
};

#endif