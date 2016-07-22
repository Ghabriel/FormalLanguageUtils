#include <queue>
#include "DFA.hpp"
#include "IndexList.hpp"

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

std::unordered_set<char> DFA::alphabet() const {
	std::unordered_set<char> result;
	for (auto& pair : states) {
		for (auto& p : pair.second.transitions) {
			result.insert(p.first);
		}
	}
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
	auto classes = getEquivalenceClasses();
	// TODO
	TRACE(classes.size());
	DFA result;
	return result;
}

DFA DFA::minimized() const {
	DFA clean = withoutUselessStates();
	return clean.withoutEquivalentStates();
}

State& DFA::operator[](const Index& index) {
	return states[index];
}

DFA& DFA::operator<<(const State& state) {
	states.insert(states.size(), state);
	if (states.size() == 1) {
		initialStateIndex = 0;
		reset();
	}
	return *this;
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
				} else {
					helper.push(partition);
					continue;
				}

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
			}
			partitions.swap(helper);
		}
	}
	return partitions;
}

void DFA::materializeErrorState() {
	// TODO
}

IndexList DFA::stateFilter(char, const IndexList&) const {
	// TODO
	return IndexList(size());
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
	return result;
}

IndexList DFA::setToList(const std::unordered_set<DFA::Index>& set) const {
	IndexList list(size());
	for (auto& index : set) {
		list.remove(index);
	}
	return !list;
}

IndexList DFA::setToList(const std::unordered_set<State>& set) const {
	IndexList list(size());
	for (auto& state : set) {
		list.remove(states[state]);
	}
	return !list;
}
