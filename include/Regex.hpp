/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef REGEX_HPP
#define REGEX_HPP

#include <climits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Regex {
public:
    Regex();
    explicit Regex(const std::string&);
    void read(char);
    bool matches(const std::string&);
    bool matches() const;
    bool aborted() const;
    void reset();

private:
    using Pattern = std::string;
    struct State {
        State() {}
        std::size_t read(char) const;
        std::unordered_map<Pattern, std::size_t> transitions;
        std::unordered_set<std::size_t> spontaneous;
    };
    struct Composition {
        Composition() : id(nextId++) {}
        static unsigned nextId;
        unsigned id = 0;
        Pattern pattern;
        char modifier = ' ';
        std::size_t ref = INT_MAX;
        bool isProtected = false;
        char groupModifier =  ' ';
        unsigned nestingLevel = 0;
    };

    std::string expression;
    std::vector<State> stateList;
    std::unordered_set<std::size_t> currentStates;

    void build(const std::vector<Composition>&, const std::vector<std::pair<unsigned, unsigned>>&);
    void expandSpontaneous(std::unordered_set<std::size_t>&) const;
    void debug(const Composition&) const;
};

#endif
