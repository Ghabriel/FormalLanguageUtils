#include "DFA.hpp"

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

void DFA::accept(State& state) {
	state.accepts = true;
}

void DFA::accept(State&& state) {
	state.accepts = true;
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
