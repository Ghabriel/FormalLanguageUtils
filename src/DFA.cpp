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

void DFA::removeDeadStates() {

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

std::unordered_set<State> DFA::getDeadStates() const {
	std::unordered_map<Index, Index> table = transitiveClosure();
	std::unordered_set<State> acceptingStates = finalStates();
	IndexList blacklist;
	for (auto& final : acceptingStates) {
		for (auto& pair : states) {
			if (table[states[state]][pair.first]) {
				blacklist.remove(pair.first);
			}
		}
	}
	removeStates(blacklist);
}
