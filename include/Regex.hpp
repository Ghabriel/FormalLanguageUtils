/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef REGEX_HPP
#define REGEX_HPP

#include <climits>
#include <deque>
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
        Pattern pattern;
        unsigned id = 0;
        int min = 1;
        int max = 1;
        unsigned nestingLevel = 0;
        bool special;
        std::vector<std::size_t> next;
        bool ready = false;
    };

    const static std::string PATTERN_OR;
    const static std::string PATTERN_CONTEXT_START;
    const static std::string PATTERN_CONTEXT_END;
    const static std::string PATTERN_WILDCARD;
    std::string expression;
    std::vector<State> stateList;
    std::unordered_set<std::size_t> currentStates;
    std::size_t acceptingState;

    void build(std::deque<Composition>&);
    void expandSpontaneous(std::unordered_set<std::size_t>&) const;
    void debug(const Composition&) const;
};

#endif
