/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef CFGREPRESENTATION_HPP
#define CFGREPRESENTATION_HPP

#include <string>
#include <functional>

struct ProductionParts {
	std::string name;
	std::vector<std::string> products;
};

class CFGRepresentation {
public:
	virtual bool isTerminal(const std::string&) const = 0;
	virtual bool isNonTerminal(const std::string&) const = 0;
	virtual std::vector<ProductionParts> decompose(const std::string&) const = 0;
	virtual std::vector<std::string> toSymbolSequence(const std::string&) const = 0;
	virtual std::string toReadableForm() const = 0;
};

#endif