#include <queue>
#include "DFA.hpp"
#include "IndexList.hpp"

const std::string DFA::errorStateName = "__ERROR__";
const std::string DFA::materializedErrorPrefix = "m__error";

void DFA::reserve(std::size_t size) {
    states.reserve(size);
}

std::size_t DFA::size() const {
    return states.size();
}

bool DFA::error() const {
    return errorState;
}

State& DFA::initialState() {
    return states[initialStateIndex];
}

void DFA::initialState(const State& state) {
    initialStateIndex = states[state];
}

State& DFA::state() {
    return states[currentState];
}

DFA& DFA::removeState(const State& state) {
    if (states.count(state) > 0) {
        Index& index = states[state];
        for (auto& pair : states) {
            State& state = pair.second;
            auto& transitions = state.transitions;
            auto it = transitions.begin();
            while (it != transitions.end()) {
                Index& to = (*it).second;
                if (to == index) {
                    it = state.transitions.erase(it);
                } else {
                    it++;
                }
            }
        }
        states.erase(index);
    }
    return *this;
}

void DFA::reset() {
    if (states.size() > 0) {
        currentState = initialStateIndex;
        errorState = false;
    } else {
        errorState = true;
    }
}

void DFA::read(const std::string& input) {
    for (char c : input) {
        read(c);
    }
}

void DFA::read(char input) {
    if (errorState) {
        return;
    }
    auto& state = states[currentState];
    if (state.transitions.count(input) == 0) {
        errorState = true;
    } else {
        currentState = state.transitions[input];
    }
}

bool DFA::accepts() const {
    return !errorState && states[currentState].accepts;
}

DFA& DFA::addTransition(const State& from, const State& to, char input) {
    states[states[from]].transitions[input] = states[to];
    return *this;
}

DFA& DFA::removeTransition(const State& from, char input) {
    states[states[from]].transitions.erase(input);
    return *this;
}

std::unordered_set<char> DFA::alphabet() const {
    std::unordered_set<char> result;
    transitionTraversal([&](const Index&, const Index&, char c) {
        result.insert(c);
    });
    return result;
}

std::unordered_set<State> DFA::finalStates() const {
    std::unordered_set<State> result;
    for (auto& pair : states) {
        if (pair.second.accepts) {
            result.insert(pair.second);
        }
    }
    return result;
}

DFA DFA::withoutDeadStates() const {
    return simplify(!getDeadStates());
}

DFA DFA::withoutUnreachableStates() const {
    return simplify(getReachableStates());
}

DFA DFA::withoutUselessStates() const {
    return simplify(~getDeadStates() & getReachableStates());
}

DFA DFA::withoutEquivalentStates() {
    DFA result;
    std::queue<IndexList> classes = getEquivalenceClasses();
    std::unordered_map<Index, Index> stateMapping;
    while (!classes.empty()) {
        IndexList eqClass = classes.front();
        classes.pop();

        Index master = eqClass.extract();
        State& masterState = states[master];
        result << masterState;
        Index trueIndex = result[masterState];
        stateMapping[master] = trueIndex;

        if (eqClass.isSet(initialStateIndex)) {
            result.initialState(masterState);
        }
        eqClass.remove(master);

        while (eqClass.count() > 0) {
            Index index = eqClass.extract();
            eqClass.remove(index);
            stateMapping[index] = trueIndex;
        }
    }

    transitionTraversal([&](const Index& from, const Index& to, char c) {
        if (stateMapping.count(from) > 0 && stateMapping.count(to) > 0) {
            result.addTransition(result[stateMapping[from]],
                                 result[stateMapping[to]],
                                 c);
        }
    });
    result.reset();
    return result;
}

DFA DFA::minimized() const {
    DFA clean = withoutUnreachableStates();
    return clean.withoutEquivalentStates();
}

bool DFA::empty() const {
    std::unordered_set<Index> reachable;
    if (size() > 0) {
        reachable = bfs(states[initialStateIndex]);
    }

    for (auto& index : reachable) {
        if (states[index].accepts) {
            return false;
        }
    }
    return true;
}

bool DFA::contains(DFA& other) {
    if (alphabet() != other.alphabet()) {
        materializeErrorState(true);
    }
    bool result = (~*this & other).empty();
    removeState(errorStateName);
    return result;
}

DFA DFA::operator~() const {
    DFA result = simplify(size());
    result.materializeErrorState();
    for (auto& pair : result.states) {
        pair.second.accepts = !pair.second.accepts;
        if (pair.second.getName() == errorStateName) {
            pair.second.name = materializedErrorPrefix + std::to_string(pair.first);
        }
    }

    bool rebuild = false;
    IndexList whitelist(size());
    for (auto& pair : result.states) {
        auto& prefix = materializedErrorPrefix;
        if (pair.second.getName().substr(0, prefix.size()) == prefix
            && !pair.second.accepts) {
            rebuild = true;
            whitelist.remove(pair.first);
        }
    }
    return rebuild ? result.simplify(whitelist) : result;
}

DFA DFA::operator&(DFA& other) {
    return productConstruction(other, [&](const std::pair<Index, Index>& pair) {
        return (states[pair.first].accepts && other.states[pair.second].accepts);
    });
}

DFA DFA::operator|(DFA& other) {
    return productConstruction(other, [&](const std::pair<Index, Index>& pair) {
        return (states[pair.first].accepts || other.states[pair.second].accepts);
    });
}

State& DFA::operator[](const Index& index) {
    return states[index];
}

DFA::Index& DFA::operator[](const State& state) {
    return states[state];
}

DFA& DFA::operator<<(const State& state) {
    states.insert(states.size(), state);
    if (states.size() == 1) {
        initialStateIndex = 0;
        reset();
    }
    return *this;
}

bool DFA::operator==(DFA& other) {
    return contains(other) && other.contains(*this);
}

void DFA::debug() const {
    Index lastIndex = -1;
    for (auto& pair : states) {
        auto& from = pair.first;
        if (from != lastIndex) {
            std::string prefix = "[" + std::to_string(from) + "] ";
            if (from == currentState) {
                prefix += "!";
            }
            if (from == initialStateIndex) {
                prefix += "->";
            }
            if (states[from].accepts) {
                prefix += "*";
            }
            ECHO(prefix + states[from].getName() + ":");
            lastIndex = from;
        }
        if (pair.second.transitions.size() == 0) {
            ECHO("\tNO TRANSITIONS");
        }
        for (auto& transition : pair.second.transitions) {
            char input = transition.first;
            auto& to = transition.second;
            ECHO("\t" + states[from].getName() + " -> " + states[to].getName() + " (" + std::string(1, input) + ")");
        }
    }
}

DFA::Index DFA::errorStateIndex() const {
    for (auto& pair : states) {
        auto& name = pair.second.getName();
        auto& prefix = materializedErrorPrefix;
        if (name == errorStateName || name.substr(0, prefix.size()) == prefix) {
            return pair.first;
        }
    }
    return -1;
}

IndexList DFA::getDeadStates() const {
    std::unordered_set<State> acceptingStates = finalStates();
    IndexList blacklist(size());
    for (auto& pair : states) {
        if (pair.second.accepts) {
            blacklist.remove(pair.first);
            continue;
        }
        std::unordered_set<Index> reachable = bfs(pair.second);
        for (auto& final : acceptingStates) {
            if (reachable.count(states[final]) > 0) {
                blacklist.remove(pair.first);
            }
        }
    }
    return blacklist;
}

IndexList DFA::getReachableStates() const {
    std::unordered_set<Index> reachable;
    if (size() > 0) {
        reachable = bfs(states[initialStateIndex]);
    }
    return setToList(reachable);
}

std::queue<IndexList> DFA::getEquivalenceClasses() {
    materializeErrorState(true);
    auto sigma = alphabet();
    auto finalSet = setToList(finalStates());

    std::queue<IndexList> partitions;
    std::queue<IndexList> helper;
    std::unordered_set<IndexList> w;

    partitions.push(finalSet);
    partitions.push(IndexList(size()) - finalSet);
    w.reserve(size());
    w.insert(finalSet);

    while (!w.empty()) {
        IndexList list = *w.begin();
        w.erase(list);
        for (char c : sigma) {
            auto predecessors = stateFilter(c, list);
            while (!partitions.empty()) {
                IndexList partition = partitions.front();
                partitions.pop();
                IndexList intersection = partition & predecessors;
                IndexList difference = partition - predecessors;
                auto intCount = intersection.count();
                auto difCount = difference.count();
                if (intCount != 0 && difCount != 0) {
                    partitions.push(intersection);
                    partitions.push(difference);
                    if (w.count(partition) > 0) {
                        w.erase(partition);
                        w.insert(intersection);
                        w.insert(difference);
                    } else {
                        if (intCount <= difCount) {
                            w.insert(intersection);
                        } else {
                            w.insert(difference);
                        }
                    }
                } else {
                    helper.push(partition);
                }
            }
            partitions.swap(helper);
        }
    }

    Index errorIndex = size() - 1;
    while (!partitions.empty()) {
        IndexList partition = partitions.front();
        partitions.pop();
        if (!partition.isSet(errorIndex)) {
            helper.push(partition);
        }
    }

    partitions.swap(helper);
    removeState(states[errorIndex]);
    return partitions;
}

void DFA::materializeErrorState(bool forced) {
    if (errorStateIndex() >= 0) {
        return;
    }
    bool created = false;
    Index errorIndex = size();
    std::unordered_set<char> sigma = alphabet();

    auto materialize = [&]() {
        *this << errorStateName;
        created = true;
    };
    if (forced) {
        materialize();
    }
    for (auto& pair : states) {
        auto& transitions = pair.second.transitions;
        for (char c : sigma) {
            if (transitions.count(c) == 0) {
                if (!created) {
                    materialize();
                }
                transitions[c] = errorIndex;
            }
        }
    }

    if (created) {
        for (char c : sigma) {
            addTransition(errorStateName, errorStateName, c);
        }
    }
}

IndexList DFA::stateFilter(char input, const IndexList& list) const {
    IndexList result(size());
    transitionTraversal([&](const Index& from, const Index& to, char c) {
        if (c == input && list.isSet(to)) {
            result.remove(from);
        }
    });
    return ~result;
}

std::unordered_set<DFA::Index> DFA::bfs(const State& state) const {
    Index origin = states[state];
    std::unordered_set<Index> result;
    std::queue<Index> queue;
    result.insert(origin);
    queue.push(origin);
    while (!queue.empty()) {
        Index current = queue.front();
        queue.pop();
        for (auto& pair : states[current].transitions) {
            if (result.count(pair.second) == 0) {
                result.insert(pair.second);
                queue.push(pair.second);
            }
        }
    }
    return result;
}

DFA DFA::simplify(const IndexList& whitelist) const {
    DFA result;
    result.reserve(size());
    for (auto& pair : states) {
        if (whitelist.isSet(pair.first)) {
            std::string newState = pair.second.getName();
            result << newState;
            if (pair.second.accepts) {
                result.accept(newState);
            }
            if (initialStateIndex == pair.first) {
                result.initialStateIndex = result.states[pair.second];
            }
        }
    }

    for (auto& pair : states) {
        if (whitelist.isSet(pair.first)) {
            auto& from = pair.second;
            for (auto& transition : from.transitions) {
                auto& to = transition.second;
                if (whitelist.isSet(to)) {
                    result.addTransition(from, states[to], transition.first);
                }
            }
        }
    }
    result.reset();
    return result;
}

IndexList DFA::setToList(const std::unordered_set<DFA::Index>& set) const {
    IndexList list(size());
    for (auto& index : set) {
        list.remove(index);
    }
    return ~list;
}

IndexList DFA::setToList(const std::unordered_set<State>& set) const {
    IndexList list(size());
    for (auto& state : set) {
        list.remove(states[state]);
    }
    return ~list;
}

DFA DFA::productConstruction(DFA& other,
    const std::function<bool(const std::pair<Index, Index>&)>& heuristic) {

    materializeErrorState(true);
    other.materializeErrorState(true);
    auto sigma1 = alphabet();
    auto sigma2 = other.alphabet();
    std::unordered_set<State> addedStates;
    std::vector<std::pair<Index, Index>> addedPairs;
    std::queue<std::pair<Index, Index>> stateList;
    std::pair<Index, Index> init = {initialStateIndex, other.initialStateIndex};
    DFA result;

    auto format = [&](const std::pair<Index, Index>& state) {
        return states[state.first].getName() + "__" + other.states[state.second].getName();
    };

    auto apply = [&](const std::pair<Index, Index>& state, char input) {
        auto& firstTransitions = states[state.first].transitions;
        auto& secondTransitions = other.states[state.second].transitions;
        Index first, second;
        first = (firstTransitions.count(input) > 0)
                ? firstTransitions[input]
                : errorStateIndex();
        second = (secondTransitions.count(input) > 0)
                 ? secondTransitions[input]
                 : other.errorStateIndex();
        return std::make_pair(first, second);
    };

    addedPairs.push_back(init);
    stateList.push(init);
    while (!stateList.empty()) {
        auto& pair = stateList.front();
        stateList.pop();

        auto before = addedStates.size();
        auto state = format(pair);
        addedStates.insert(state);
        if (addedStates.size() != before) {
            result << state;
            for (char c : sigma1) {
                auto newPair = apply(pair, c);
                addedPairs.push_back(newPair);
                stateList.push(newPair);
            }
            for (char c : sigma2) {
                auto newPair = apply(pair, c);
                addedPairs.push_back(newPair);
                stateList.push(newPair);
            }
        }
    }

    for (auto& pair : addedPairs) {
        std::string state = format(pair);
        for (char c : sigma1) {
            result.addTransition(state, format(apply(pair, c)), c);
        }
        for (char c : sigma2) {
            result.addTransition(state, format(apply(pair, c)), c);
        }
        if (heuristic(pair)) {
            result.accept(state);
        }
    }

    removeState(errorStateName);
    other.removeState(errorStateName);
    return result;
}

void DFA::transitionTraversal(
    const std::function<void(const DFA::Index&, const DFA::Index&, char)>& fn) const {

    for (auto& pair : states) {
        for (auto& transition : pair.second.transitions) {
            fn(pair.first, transition.second, transition.first);
        }
    }
}
