/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef REGEX_HPP
#define REGEX_HPP

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
        explicit State(Regex* inner) : inner(inner) {}
        std::size_t read(char) const;
        std::unordered_map<Pattern, std::size_t> transitions;
        std::unordered_set<std::size_t> spontaneous;
        std::unique_ptr<Regex> inner;
    };
    struct Composition {
        Pattern pattern;
        char modifier = ' ';
        std::vector<Composition> inner;
    };

    std::string expression;
    std::vector<State> stateList;
    std::unordered_set<std::size_t> currentStates;

    void build(const std::vector<Composition>&);
    void expandSpontaneous(std::unordered_set<std::size_t>&) const;
    void debug(const Composition&) const;
};

#endif
