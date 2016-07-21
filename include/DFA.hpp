#ifndef DFA_HPP
#define DFA_HPP

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "utils.hpp"

class DFA;

class State {
    friend class DFA;
public:
    State(const char* name) : name(name) {}
    State(const std::string& name) : State(name.c_str()) {}
    State() : State("") {}
    const std::string& getName() const { return name; }
    bool operator==(const State& other) const { return name == other.name; }
    bool operator<(const State& other) const { return name < other.name; }

private:
    std::string name;
    bool accepts;
    std::unordered_map<char, utils::Index> transitions;
};

inline bool operator==(const char* lhs, const State& rhs) {
    return std::string(lhs) == rhs.getName();
}

namespace std {
    template<>
    struct hash<State> {
        std::size_t operator()(const State& state) const {
            return std::hash<std::string>()(state.getName());
        }
    };
}

class DFA {
public:
    using Index = utils::Index;

    // Prepares this DFA to hold at least n elements
    void reserve(std::size_t);

    // Returns the number of states of this DFA.
    std::size_t size() const;

    // Checks if this DFA is in an error state.
    bool error() const;

    // Returns the initial state of this DFA. Undefined behavior
    // if it has no states (check with size()).
    State& initialState();

    // Sets the initial state.
    void initialState(const State&);

    // Marks a state as final.
    void accept(State&);
    void accept(State&&);

    // Returns the current state of this DFA. Undefined behavior
    // if it has no states (check with size()). Note that if this
    // DFA is in an error state (check with error()), the
    // state returned by this method is the last non-error state
    // that it has been in, not the current state.
    State& state();

    // Resets this DFA to its initial state or, if it has no
    // states, puts it in the error state.
    void reset();

    // Reads an input, transitioning according to the characters
    // it contains.
    void read(const std::string&);

    // Reads a character, transitioning according to it.
    void read(char);

    // Checks if this DFA is in a final state.
    bool accepts() const;

    // Adds a transition to this DFA, returning itself to allow chaining.
    DFA& addTransition(const State&, const State&, char);

    // Returns all characters used in transitions in this DFA.
    std::unordered_set<char> alphabet() const;

    // Returns a state, given its index.
    State& operator[](const Index&);

    // Adds a state to this DFA, returning itself to allow chaining.
    DFA& operator<<(const State&);

private:
    utils::bimap<Index, State> states;
    Index currentState;
    Index initialStateIndex;
    bool errorState = true;
};

#endif