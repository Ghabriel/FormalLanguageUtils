/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <algorithm>
#include <cassert>
#include <stack>
#include <queue>
#include "Regex.hpp"
#include "utils.hpp"

Regex::Regex() {}

Regex::Regex(const std::string& expr) : expression(expr) {
    bool escape = false;
    std::size_t i = 0;
    std::size_t length = expr.size();
    std::vector<std::vector<Composition>> contexts;
    contexts.push_back(std::vector<Composition>());
    std::vector<std::pair<unsigned, unsigned>> branches;
    bool contextChange = false;

    std::queue<char> extraSymbols;
    auto next = [&]() {
        if (extraSymbols.empty()) {
            i++;
        } else {
            extraSymbols.pop();
        }
    };
    while (i < length || !extraSymbols.empty()) {
        char c = (extraSymbols.empty()) ? expr[i] : extraSymbols.front();
        std::string buffer;
        buffer = c;
        if (!escape) {
            bool skip = true;
            switch (c) {
                case '*':
                case '+':
                case '?': {
                    auto& prevComp = contexts.back().back();
                    assert(prevComp.pattern.size() > 0);
                    assert((!prevComp.isProtected && prevComp.modifier == ' ')
                        || (prevComp.isProtected && prevComp.groupModifier == ' '));
                    if (prevComp.isProtected) {
                        prevComp.groupModifier = c;                    
                    } else {
                        prevComp.modifier = c;
                    }
                    break;
                }
                case '{': {
                    assert(false);
                    // auto& prevComp = contexts.back().back();
                    // assert(prevComp.pattern.size() > 0);
                    // assert((!prevComp.isProtected && prevComp.modifier == ' ')
                    //     || (prevComp.isProtected && prevComp.groupModifier == ' '));

                    // int start = -1;
                    // int end = -1;
                    // i++;
                    // buffer.clear();
                    // while (expr[i] != '}' && i < length) {
                    //     buffer += expr[i];
                    //     if (expr[i] == ',') {
                    //         start = stoi(buffer);
                    //         buffer.clear();
                    //         continue;
                    //     }
                    //     i++;
                    // }
                    // assert(i < length);
                    // end = stoi(buffer);
                    // if (start < 0) {
                    //     start = end;
                    // }

                    // assert(start >= 0 && end > 0);
                    // for (unsigned index = 1; index < start; index++) {
                    //     extraSymbols.push(prevComp);
                    // }
                    break;
                }
                case '|':
                    branches.push_back(std::make_pair(contexts.back().front().id, nextCompositionId));
                    break;
                case '(':
                    contexts.push_back(std::vector<Composition>());
                    break;
                case ')': {
                    assert(contexts.size() > 1);
                    std::size_t nextIndex = contexts[contexts.size() - 2].size();
                    for (auto& comp : contexts.back()) {
                        contexts[contexts.size() - 2].push_back(std::move(comp));
                    }
                    contexts.pop_back();
                    contexts.back().back().ref = nextIndex;
                    contexts.back().back().isProtected = true;
                    contextChange = true;
                    break;
                }
                case '[': {
                    buffer.clear();
                    while (expr[i] != ']' && i < length) {
                        buffer += expr[i];
                        i++;
                    }
                    assert(i < length);
                    buffer += ']';
                    skip = false;
                    break;
                }
                case '\\':
                    escape = true;
                    break;
                case '.':
                    buffer = "[[";
                    skip = false;
                    break;
                default:
                    skip = false;
            }

            if (skip) {
                next();
                continue;
            }
        }

        escape = false;
        Composition comp(nextCompositionId++);
        comp.pattern = std::move(buffer);
        comp.contextChange = contextChange;
        contextChange = false;
        contexts.back().push_back(std::move(comp));
        next();
    }

    assert(contexts.size() == 1);

    for (i = 0; i < contexts.size(); i++) {
        for (std::size_t j = 0; j < contexts[i].size(); j++) {
            debug(contexts[i][j]);
            ECHO("");
        }
        ECHO("");
    }
    // assert(false);

    build(contexts.back(), branches);
}

void Regex::build(const std::vector<Regex::Composition>& entities,
    const std::vector<std::pair<unsigned, unsigned>>& branches) {

    std::unordered_map<std::size_t, std::size_t> entityToState;
    stateList.push_back(State());
    // Creates simple transitions, ignoring modifiers
    for (std::size_t i = 0; i < entities.size(); i++) {
        auto& entity = entities[i];
        for (auto& pair : branches) {
            if (/*pair.first == entity.id || */pair.second == entity.id) {
                stateList.push_back(State());
            }
        }

        std::size_t lastIndex = stateList.size() - 1;
        entityToState[i] = lastIndex;
        stateList.back().transitions[entity.pattern] = stateList.size();
        stateList.push_back(State());
    }

    // Creates transitions that involve modifiers
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

        std::size_t refIndex = entityToState[entity.ref];
        switch (entity.groupModifier) {
            case '*':
                stateList[refIndex].spontaneous.insert(stateIndex + 1);
                stateList[stateIndex + 1].spontaneous.insert(refIndex);
                break;
            case '+':
                stateList[stateIndex + 1].spontaneous.insert(refIndex);
                break;
            case '?':
                stateList[refIndex].spontaneous.insert(stateIndex + 1);
                break;
        }
    }

    // Creates transitions that involve the | operator
    for (auto& pair : branches) {
        std::size_t s1 = entityToState[pair.first];
        std::size_t s2 = entityToState[pair.second];
        stateList[s1].spontaneous.insert(s2);
        // stateList[s1 - 1].spontaneous.insert(s1);
        // stateList[s1 - 1].spontaneous.insert(s2);

        std::size_t i = pair.second + 1;
        while (i < entities.size()) {
            if (entities[i].contextChange && entities[i].ref > i) {
                break;
            }
            i++;
        }
        std::size_t targetIndex;
        if (i == entities.size()) {
            targetIndex = stateList.size() - 1;
        } else {
            targetIndex = entityToState[i];
        }
        std::size_t delta = pair.second - pair.first;
        stateList[s1 + delta].spontaneous.insert(targetIndex);
    }

    acceptingState = stateList.size() - 1;

    // Partitions states that have back-references and forward-references
    std::unordered_map<std::size_t, std::size_t> stateMapping;
    for (std::size_t i = 0; i < stateList.size(); i++) {
        State& state = stateList[i];
        bool hasBackReference = false;
        bool hasForwardReference = false;
        std::unordered_set<std::size_t> forwardIndexes;
        for (auto& pair : state.transitions) {
            if (stateMapping.count(pair.second) > 0) {
                pair.second = stateMapping[pair.second];
            }
        }

        for (auto& pair : state.transitions) {
            if (stateMapping.count(pair.second) > 0) {
                state.transitions[pair.first] = stateMapping[pair.second];
            }
        }

        std::unordered_set<std::size_t> newTransitions;
        newTransitions.reserve(state.spontaneous.size());
        for (auto& index : state.spontaneous) {
            if (stateMapping.count(index) > 0) {
                newTransitions.insert(stateMapping[index]);
            } else {
                newTransitions.insert(index);
            }
        }
        state.spontaneous.swap(newTransitions);

        for (auto index : state.spontaneous) {
            if (index < i) {
                hasBackReference = true;
            }
            if (index > i) {
                hasForwardReference = true;
                forwardIndexes.insert(index);
            }
            if (hasBackReference && hasForwardReference) {
                break;
            }
        }

        if (hasBackReference && hasForwardReference) {
            State newState;
            newState.transitions = state.transitions;
            newState.spontaneous = std::move(forwardIndexes);
            stateMapping.insert(std::make_pair(i, stateList.size()));
            stateList.push_back(std::move(newState));
        }
    }

    reset();

    // ECHO("-----");
    // for (auto& pair : entityToState) {
    //     TRACE(pair.first);
    //     TRACE(pair.second);
    // }

    for (std::size_t i = 0; i < stateList.size(); i++) {
        ECHO(std::to_string(i) + ":");
        for (auto& pair : stateList[i].transitions) {
            ECHO("\t" + pair.first + " -> " + std::to_string(pair.second));
        }
        for (auto& index : stateList[i].spontaneous) {
            ECHO("\t& -> " + std::to_string(index));
        }
    }
    TRACE(acceptingState);
}

void Regex::read(char c) {
    std::unordered_set<std::size_t> newStates;
    for (std::size_t index : currentStates) {
        std::size_t newIndex = stateList[index].read(c);
        if (newIndex < INT_MAX) {
            newStates.insert(newIndex);
        }
    }
    expandSpontaneous(newStates);
    // TRACE(c);
    // std::vector<std::size_t> container(newStates.begin(), newStates.end());
    // std::sort(container.begin(), container.end());
    // TRACE_IT(container);
    // ECHO("");
    currentStates = std::move(newStates);
}

bool Regex::matches(const std::string& input) {
    reset();
    for (char c : input) {
        read(c);
    }
    return matches();
}

bool Regex::matches() const {
    return currentStates.count(acceptingState) > 0;
}

bool Regex::aborted() const {
    return currentStates.size() == 0;
}

void Regex::reset() {
    currentStates.clear();
    currentStates.insert(0);
    expandSpontaneous(currentStates);
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
    ECHO("[" + std::to_string(comp.id) + "]");
    TRACE(comp.pattern);
    TRACE(comp.modifier);
    TRACE(comp.ref);
    TRACE(comp.isProtected);
    TRACE(comp.groupModifier);
    TRACE(comp.contextChange);
}

std::size_t Regex::State::read(char c) const {
    for (auto& pair : transitions) {
        bool ok = false;
        if (pair.first.front() == '[' && pair.first.back() == ']') {
            char buffer = '\0';
            bool validBuffer = false;
            bool intervalMode = false;
            std::size_t length = pair.first.size();
            for (std::size_t i = 1; i < length - 1; i++) {
                char s = pair.first[i];
                if (s == '-' && validBuffer) {
                    validBuffer = false;
                    intervalMode = true;
                    continue;
                }

                if (intervalMode) {
                    if (buffer <= c && c <= s) {
                        ok = true;
                        break;
                    }
                    intervalMode = false;
                    continue;
                }

                if (!validBuffer) {
                    buffer = s;
                    validBuffer = true;
                    continue;
                }

                if (c == buffer) {
                    ok = true;
                    break;
                }
                buffer = s;
            }

            if (intervalMode) {
                buffer = '-';
                validBuffer = true;
            }

            if (validBuffer && c == buffer) {
                ok = true;
            }
        } else if (pair.first.front() == '[' && pair.first.back() == '[') {
            ok = true;
        } else if (c == pair.first.front()) {
            ok = true;
        }

        if (ok) {
            return pair.second;
        }
    }
    return INT_MAX;
}
