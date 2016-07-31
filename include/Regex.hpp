/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef REGEX_HPP
#define REGEX_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Regex {
public:
    Regex(const std::string&);
    bool matches(const std::string&);

private:
    using Pattern = std::string;
    struct State {
        std::size_t read(char) const;
        std::unordered_map<Pattern, std::size_t> transitions;
        std::unordered_set<std::size_t> spontaneous;
    };
    struct Composition {
        Pattern pattern;
        char modifier = ' ';
        std::vector<Composition> inner;
    };

    std::string expression;
    std::vector<State> stateList;

    void build(const std::vector<Composition>&);
    void expandSpontaneous(std::unordered_set<std::size_t>&) const;
    void debug(const Composition&) const;
};

#endif
