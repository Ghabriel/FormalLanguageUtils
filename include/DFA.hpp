#ifndef DFA_HPP
#define DFA_HPP

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "utils.hpp"

class DFA;
class IndexList;

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
    bool accepts = false;
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

    // Marks a set of states as final.
    template<typename... Args>
    void accept(State& state, Args... args) {
        state.accepts = true;
        accept(args...);
    }
    template<typename... Args>
    void accept(State&& state, Args... args) {
        states[states[state]].accepts = true;
        accept(args...);
    }

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

    // Returns a set containing all final states of this DFA.
    std::unordered_set<State> finalStates() const;

    // Returns a DFA equivalent to this one, but without dead states.
    // Complexity: O(n(n + m))
    DFA withoutDeadStates() const;

    // Returns a DFA equivalent to this one, but without unreachable states.
    // Complexity: O(n + m)
    DFA withoutUnreachableStates() const;

    // Returns a DFA equivalent to this one, but without useless states.
    // Has the same effect as withoutDeadStates().withoutUnreachableStates()
    // but is faster.
    DFA withoutUselessStates() const;

    // TODO: find a way to make this const
    DFA withoutEquivalentStates();

    // Returns the minimized form of this DFA.
    DFA minimized() const;

    // Returns a state, given its index.
    State& operator[](const Index&);

    // Adds a state to this DFA, returning itself to allow chaining.
    DFA& operator<<(const State&);

private:
    utils::bimap<Index, State> states;
    Index currentState;
    Index initialStateIndex;
    bool errorState = true;

    void accept() {}

    // Returns an IndexList where each bit is 1 if the state is dead,
    // 0 otherwise.
    // Complexity: O(n(n + m))
    IndexList getDeadStates() const;

    // Returns an IndexList where each bit is 1 if the state is reachable,
    // 0 otherwise.
    // Complexity: O(n + m)
    IndexList getReachableStates() const;

    // Returns the equivalence classes of this DFA.
    // TODO: find a way to make this const
    std::queue<IndexList> getEquivalenceClasses();

    void materializeErrorState();

    IndexList stateFilter(char, const IndexList&) const;

    // Executes breadth-first search on a state, returning a set
    // containing all states that are reachable from it.
    // Complexity: O(m)
    std::unordered_set<Index> bfs(const State&) const;

    // Receives a list of valid state indexes and returns a DFA
    // equal to this one but only using the allowed states.
    // Complexity: O(n + m)
    DFA simplify(const IndexList&) const;

    IndexList setToList(const std::unordered_set<Index>&) const;
    IndexList setToList(const std::unordered_set<State>&) const;
};

#endif