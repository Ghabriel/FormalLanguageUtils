/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include <stack>
#include <queue>
#include "Regex.hpp"
#include "utils.hpp"

unsigned Regex::Composition::nextId = 0;

Regex::Regex() {}

Regex::Regex(const std::string& expr) : expression(expr) {
    bool escape = false;
    std::size_t i = 0;
    std::size_t length = expr.size();
    std::vector<std::vector<Composition>> contexts;
    contexts.push_back(std::vector<Composition>());
    std::vector<std::pair<unsigned, unsigned>> branches;
    unsigned nestingLevel = 0;
    while (i < length) {
        char c = expr[i];
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
                case '|':
                    branches.push_back(std::make_pair(contexts.back().front().id, Composition::nextId));
                    // auto currContext = contexts.back();
                    // contexts.back().clear();
                    // contexts.back().push_back(Composition());
                    // auto& currComposition = contexts.back().back();
                    // currComposition.modifier = '|';
                    // currComposition.ref = contexts.back().size();
                    // currComposition.rightRef = currComposition.ref + currContext.size();
                    // for (auto& comp : currContext) {
                    //     contexts.back().push_back(comp);
                    // }
                    break;
                case '(':
                    contexts.push_back(std::vector<Composition>());
                    nestingLevel++;
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
                    nestingLevel--;
                    break;
                }
                case '[': {
                    std::string buffer;
                    while (expr[i] != ']' && i < length) {
                        buffer += expr[i];
                        i++;
                    }
                    assert(i < length);
                    buffer += ']';
                    Composition comp;
                    comp.pattern = std::move(buffer);
                    comp.nestingLevel = nestingLevel;
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
        comp.nestingLevel = nestingLevel;
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

    // for (auto& pair : branches) {
    //     TRACE(pair.first);
    //     TRACE(pair.second);
    //     ECHO("");
    // }
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
            if (pair.second == entity.id) {
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

        switch (entity.groupModifier) {
            case '*':
                stateList[entity.ref].spontaneous.insert(stateIndex + 1);
                stateList[stateIndex + 1].spontaneous.insert(entity.ref);
                break;
            case '+':
                stateList[stateIndex + 1].spontaneous.insert(entity.ref);
                break;
            case '?':
                stateList[entity.ref].spontaneous.insert(stateIndex + 1);
                break;
        }
    }

    // Creates transitions that involve the operator |
    for (auto& pair : branches) {
        std::size_t s1 = entityToState[pair.first];
        std::size_t s2 = entityToState[pair.second];
        stateList[s1].spontaneous.insert(s2);

        unsigned nestingLevel = entities[pair.second].nestingLevel;
        std::size_t i = pair.second + 1;
        while (i < entities.size() && entities[i].nestingLevel >= nestingLevel) {
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

    reset();

    // ECHO("-----");
    // for (auto& pair : entityToState) {
    //     TRACE(pair.first);
    //     TRACE(pair.second);
    // }

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

void Regex::read(char c) {
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

bool Regex::matches(const std::string& input) {
    reset();
    for (char c : input) {
        read(c);
    }
    return matches();
}

bool Regex::matches() const {
    return currentStates.count(stateList.size() - 1) > 0;
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
    TRACE(comp.nestingLevel);
}

std::size_t Regex::State::read(char c) const {
    for (auto& pair : transitions) {
        bool ok = false;
        if (pair.first[0] == '[') {
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
        } else if (c == pair.first[0]) {
            ok = true;
        }

        if (ok) {
            return pair.second;
        }
    }
    return INT_MAX;
}
