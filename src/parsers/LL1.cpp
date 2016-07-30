#include "parsers/LL1.hpp"
#include "utils.hpp"

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
				follow.insert("$");
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

	for (auto& pair : table) {
		TRACE(pair.first);
		for (auto& p : pair.second) {
			ECHO(p.first + " -> " + std::to_string(p.second));
		}
	}
}

ParseResults parser::LL1::parse(const std::string&) {
	ParseResults results;
	return results;
}

bool parser::LL1::canParse() const {
	return !conflict;
}
 