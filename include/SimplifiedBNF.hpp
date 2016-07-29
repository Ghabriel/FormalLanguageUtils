/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef SIMPLIFIED_BNF_HPP
#define SIMPLIFIED_BNF_HPP

class SimplifiedBNF : public CFGRepresentation {
	bool isTerminal(const std::string&) const override {
		
	}
};



// struct ProductionParts {
// 	std::string name;
// 	std::vector<std::string> products;
// };

// class CFGRepresentation {
// public:
// 	virtual bool isTerminal(const std::string&) const = 0;
// 	virtual bool isNonTerminal(const std::string&) const = 0;
// 	virtual ProductionParts decompose(const std::string&) const = 0;
// 	virtual std::string toReadableForm() const = 0;
// };

#endif