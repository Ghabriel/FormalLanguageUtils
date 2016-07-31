/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include <climits>
#include <queue>
#include "Regex.hpp"
#include "utils.hpp"

Regex::Regex(const std::string& expr) : expression(expr) {
    bool escape = false;
    std::size_t i = 0;
    std::size_t length = expr.size();
    std::vector<std::vector<Composition>> contexts;
    contexts.push_back(std::vector<Composition>());
    while (i < length) {
        char c = expr[i];
        if (!escape) {
            bool skip = true;
            switch (c) {
                case '*':
                case '+':
                case '?':
                    assert(contexts.back().back().pattern.size() > 0);
                    assert(contexts.back().back().modifier == ' ');
                    contexts.back().back().modifier = c;
                    break;
                case '(':
                    contexts.push_back(std::vector<Composition>());
                    break;
                case ')': {
                    assert(contexts.size() > 1);
                    Composition comp;
                    comp.inner = std::move(contexts.back());
                    contexts.pop_back();
                    contexts.back().push_back(std::move(comp));
                    break;
                }
                default:
                    skip = false;
            }

            if (skip) {
                i++;
                continue;
            }
        }

        Composition comp;
        comp.pattern = c;
        contexts.back().push_back(std::move(comp));
        i++;
    }

    assert(contexts.size() == 1);

    // for (i = 0; i < contexts.size(); i++) {
    //     for (std::size_t j = 0; j < contexts[i].size(); j++) {
    //         debug(contexts[i][j]);
    //         ECHO("");
    //     }
    //     ECHO("");
    // }

    if (contexts.size() > 0) {
        build(contexts.back());
    }
}

bool Regex::matches(const std::string& input) {
    std::unordered_set<std::size_t> currentStates;
    currentStates.insert(0);
    expandSpontaneous(currentStates);

    std::size_t length = input.size();
    for (std::size_t i = 0; i < length; i++) {
        char c = input[i];
        std::unordered_set<std::size_t> newStates;
        for (std::size_t index : currentStates) {
            std::size_t newIndex = stateList[index].read(c);
            if (newIndex < INT_MAX) {
                newStates.insert(newIndex);
            }
        }
        expandSpontaneous(newStates);
        currentStates = std::move(newStates);
    }

    return currentStates.count(stateList.size() - 1) > 0;
}

void Regex::build(const std::vector<Regex::Composition>& entities) {
    std::unordered_map<std::size_t, std::size_t> entityToState;
    stateList.push_back(State());
    // Creates simple transitions, ignoring modifiers
    for (std::size_t i = 0; i < entities.size(); i++) {
        auto& entity = entities[i];
        if (entity.inner.size() > 0) {
            // TODO
            assert(false);
        }
        entityToState[i] = stateList.size() - 1;
        stateList.back().transitions[entity.pattern] = stateList.size();
        stateList.push_back(State());
    }

    // Creates missing transitions, considering modifiers
    for (std::size_t i = 0; i < entities.size(); i++) {
        auto& entity = entities[i];
        std::size_t stateIndex = entityToState[i];
        switch (entity.modifier) {
            case '*':
                stateList[stateIndex].spontaneous.insert(stateIndex + 1);
                stateList[stateIndex + 1].spontaneous.insert(stateIndex);
                break;
            case '+':
                stateList[stateIndex + 1].spontaneous.insert(stateIndex);
                break;
            case '?':
                stateList[stateIndex].spontaneous.insert(stateIndex + 1);
                break;
        }
    }

    // for (std::size_t i = 0; i < stateList.size(); i++) {
    //     ECHO(std::to_string(i) + ":");
    //     for (auto& pair : stateList[i].transitions) {
    //         ECHO("\t" + pair.first + " -> " + std::to_string(pair.second));
    //     }
    //     for (auto& index : stateList[i].spontaneous) {
    //         ECHO("\t& -> " + std::to_string(index));
    //     }
    // }
}

void Regex::expandSpontaneous(std::unordered_set<std::size_t>& states) const {
    std::queue<std::size_t> queue;
    for (auto& state : states) {
        queue.push(state);
    }

    while (!queue.empty()) {
        std::size_t state = queue.front();
        queue.pop();
        for (std::size_t index : stateList[state].spontaneous) {
            if (states.count(index) == 0) {
                states.insert(index);
                queue.push(index);
            }
        }
    }
}

void Regex::debug(const Composition& comp) const {
    TRACE(comp.pattern);
    TRACE(comp.modifier);
    TRACE(comp.inner.size());
    if (comp.inner.size() > 0) {
        ECHO("[IN]");
        for (auto& c : comp.inner) {
            debug(c);
        }
        ECHO("[OUT]");
    }
}

std::size_t Regex::State::read(char c) const {
    for (auto& pair : transitions) {
        // TODO: proper pattern check
        if (std::string(1, c) == pair.first) {
            return pair.second;
        }
    }
    return INT_MAX;
}
