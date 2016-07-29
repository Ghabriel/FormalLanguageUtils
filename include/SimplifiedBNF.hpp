/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef SIMPLIFIED_BNF_HPP
#define SIMPLIFIED_BNF_HPP

#include <regex>

class SimplifiedBNF : public CFGRepresentation {
public:
	bool isTerminal(const std::string& symbol) const override {
	    return symbol[0] != '<';
	}

	bool isNonTerminal(const std::string& symbol) const override {
		return symbol.front() == '<' && symbol.back() == '>';
	}

	std::vector<ProductionParts> decompose(const std::string& group) const override {
	    const static std::regex valid("(<[A-Za-z0-9_']+>) ?::= ?(.*)");
	    std::smatch matches;
	    std::regex_match(group, matches, valid);
	    assert(matches.size() > 0);
	    std::string buffer;
	    std::string productions = matches[2];
	    std::vector<ProductionParts> result;
	    for (char c : productions) {
	        if (c == '|') {
	            add(result, matches[1], buffer);
	            buffer.clear();
	        } else {
	            buffer += c;
	        }
	    }
	    add(result, matches[1], buffer);
	    return result;
	}

	std::string toReadableForm(const std::string& name,
		const std::vector<std::string>& products) const override {

	    std::string result = name + " ::= ";
	    for (auto& symbol : products) {
	        result += symbol;
	    }
	    return result;
	}

private:
	void add(std::vector<ProductionParts>& result,
		const std::string& name, const std::string& buffer) const {

		ProductionParts production;
		production.name = name;
		production.products = toSymbolSequence(buffer);
		result.push_back(std::move(production));
	}
};

#endif