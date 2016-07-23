#include <queue>
#include "DFA.hpp"
#include "IndexList.hpp"

const std::string DFA::errorStateName = "__ERROR__";

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
	return states[currentState].accepts;
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
		IndexList& eqClass = classes.front();
		classes.pop();

		Index master = eqClass.extract();
		eqClass.remove(master);

		State& masterState = states[master];
		result << masterState;
		Index trueIndex = result[masterState];
		stateMapping[master] = trueIndex;

		while (eqClass.count() > 0) {
			Index index = eqClass.extract();
			eqClass.remove(index);
			stateMapping[index] = trueIndex;
			// ECHO("[REPLACE] " + states[index].getName() + " to " + masterState.getName());
		}
	}

	transitionTraversal([&](const Index& from, const Index& to, char c) {
		result.addTransition(result[stateMapping[from]],
							 result[stateMapping[to]],
							 c);
	});
	return result;
}

DFA DFA::minimized() const {
	DFA clean = withoutUselessStates();
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

DFA DFA::operator~() const {
	DFA result = simplify(IndexList(size()));
	result.materializeErrorState();
	for (auto& pair : result.states) {
		pair.second.accepts = !pair.second.accepts;
		if (pair.second.getName() == errorStateName) {
			pair.second.name = "m__error" + std::to_string(pair.first);
		}
	}
	return result;
}

DFA DFA::operator&(DFA& other) {
    materializeErrorState();
    other.materializeErrorState();
    auto sigma1 = alphabet();
    auto sigma2 = other.alphabet();
    std::unordered_set<State> addedStates;
    std::vector<std::pair<Index, Index>> addedPairs;
    std::queue<std::pair<Index, Index>> stateList;
    std::pair<Index, Index> init = {initialStateIndex, other.initialStateIndex};
    DFA result;

    auto format = [&](const std::pair<Index, Index>& state) {
        return states[state.first].getName() + "__" + states[state.second].getName();
    };

    auto apply = [&](const std::pair<Index, Index>& state, char input) {
        return std::make_pair(states[state.first].transitions[input],
                              other.states[state.second].transitions[input]);
    };

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
        if (states[pair.first].accepts && other.states[pair.second].accepts) {
            result.accept(state);
        }
    }
    return result;
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

void DFA::debug() const {
	Index lastIndex = -1;
	transitionTraversal([&](const Index& from, const Index& to, char input) {
		if (from != lastIndex) {
			std::string prefix;
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
		ECHO("\t" + states[from].getName() + " -> " + states[to].getName() + " (" + std::string(1, input) + ")");
	});
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
	materializeErrorState();
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
				if (intersection != 0 && difference != 0) {
					partitions.push(intersection);
					partitions.push(difference);
					if (w.count(partition) > 0) {
						w.erase(partition);
						w.insert(intersection);
						w.insert(difference);
					} else {
						if (intersection.count() <= difference.count()) {
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

void DFA::materializeErrorState() {
	if (states.count(errorStateName) != 0) {
		return;
	}
	bool created = false;
	Index errorIndex = size();
	std::unordered_set<char> sigma = alphabet();
	for (auto& pair : states) {
		auto& transitions = pair.second.transitions;
		for (char c : sigma) {
			if (transitions.count(c) == 0) {
				if (!created) {
					*this << State(errorStateName);
					created = true;
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
			result << pair.second;
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

void DFA::transitionTraversal(
	const std::function<void(const DFA::Index&, const DFA::Index&, char)>& fn) const {

	for (auto& pair : states) {
		for (auto& transition : pair.second.transitions) {
			fn(pair.first, transition.second, transition.first);
		}
	}
}
