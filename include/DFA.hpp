/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */

#ifndef DFA_HPP
#define DFA_HPP

#include <queue>
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
    // Currently, must be signed.
    using Index = long;

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

    // Marks a set of states as final, returning this DFA to allow chaining.
    template<typename... Args>
    DFA& accept(State& state, Args... args) {
        state.accepts = true;
        accept(args...);
        return *this;
    }
    template<typename... Args>
    DFA& accept(State&& state, Args... args) {
        states[states[state]].accepts = true;
        accept(args...);
        return *this;
    }

    // Returns the current state of this DFA. Undefined behavior
    // if it has no states (check with size()). Note that if this
    // DFA is in an error state (check with error()), the
    // state returned by this method is the last non-error state
    // that it has been in, not the current state.
    State& state();

    // Removes a state of this DFA, also removing all transitions
    // involving it. Returns this DFA to allow chaining.
    // Complexity: O(m)
    DFA& removeState(const State&);

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

    // Removes a transition from this DFA, returning itself to allow chaining.
    DFA& removeTransition(const State&, char);

    // Returns all characters used in transitions in this DFA.
    // Complexity: O(m)
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
    // Complexity: O(n(n + m))
    DFA withoutUselessStates() const;

    // Returns a DFA equivalent to this one, but without equivalent states.
    // Complexity: O(kn.log n + m), where k is the size of the alphabet
    DFA withoutEquivalentStates();

    // Returns the minimized form of this DFA.
    // Complexity: O(kn.log n + m), where k is the size of the alphabet
    DFA minimized() const;

    // Checks if this DFA is empty, i.e, doesn't accept anything.
    // Complexity: O(m)
    bool empty() const;

    // Checks if the language of this DFA contains the language
    // of another DFA.
    // Complexity: O(k1.n1 + k2.n2 + (m1 + m2).(k1 + k2))
    bool contains(DFA&);
    bool contains(DFA&&);

    // Adds a state to this DFA representing the error state,
    // making this DFA complete. Note that, if this DFA is already complete,
    // the error state is added if and only if forced is true.
    // Complexity: O(kn + m), where k is the size of the alphabet
    void materializeErrorState(bool forced = false);

    // Returns the complement of this DFA.
    // Complexity: O(kn + m), where k is the size of the alphabet
    DFA operator~() const;

    // Returns the intersection between this and another DFA.
    // Complexity: O((m1 + m2).(k1 + k2))
    DFA operator&(DFA&);
    DFA operator&(DFA&&);

    // Returns the union between this and another DFA.
    // Complexity: O((m1 + m2).(k1 + k2))
    DFA operator|(DFA&);
    DFA operator|(DFA&&);

    // Checks if two DFAs are equal.
    // Complexity: O((m1 + m2).(k1 + k2))
    bool operator==(DFA&);
    bool operator==(DFA&&);

    // Returns a state, given its index.
    State& operator[](const Index&);

    // Returns an index, given its state.
    Index& operator[](const State&);

    // Adds a state to this DFA, returning itself to allow chaining.
    DFA& operator<<(const State&);

    // Prints all relevant information about this DFA.
    void debug() const;

private:
    utils::bimap<Index, State> states;
    Index currentState;
    Index initialStateIndex;
    bool errorState = true;
    const static std::string errorStateName;
    const static std::string materializedErrorPrefix;

    void accept() {}

    // Returns the index of the error state, or -1 if it's not materialized.
    // Complexity: O(n)
    Index errorStateIndex() const;

    // Returns an IndexList where each bit is 1 if the state is dead,
    // 0 otherwise.
    // Complexity: O(n(n + m))
    IndexList getDeadStates() const;

    // Returns an IndexList where each bit is 1 if the state is reachable,
    // 0 otherwise.
    // Complexity: O(n + m)
    IndexList getReachableStates() const;

    // Returns the equivalence classes of this DFA.
    // Complexity: O(kn.log n), where k is the size of the alphabet
    std::queue<IndexList> getEquivalenceClasses();

    // Returns the set of states that, when reading a given input,
    // go to a state of a given set.
    // Complexity: O(m)
    IndexList stateFilter(char, const IndexList&) const;

    // Executes breadth-first search on a state, returning a set
    // containing all states that are reachable from it.
    // Complexity: O(m)
    std::unordered_set<Index> bfs(const State&) const;

    // Receives a list of valid state indexes and returns a DFA
    // equal to this one but only using the allowed states.
    // Complexity: O(n + m)
    DFA simplify(const IndexList&) const;

    // Converts an unordered set to an IndexList
    IndexList setToList(const std::unordered_set<Index>&) const;
    IndexList setToList(const std::unordered_set<State>&) const;

    // Builds the product construction between this and another DFA,
    // using a given heuristic to decide which states are final.
    // Complexity: O((m1 + m2).(k1 + k2))
    DFA productConstruction(DFA&, const std::function<bool(const std::pair<Index, Index>&)>&);

    // Traverses over all transitions of this DFA, applying a callback on each.
    void transitionTraversal(const std::function<void(const Index&, const Index&, char)>&) const;
};

#endif