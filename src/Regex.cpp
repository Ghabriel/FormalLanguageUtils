/* created by Ghabriel Nunes <ghabriel.nunes@gmail.com> [2016] */
#include <cassert>
#include <queue>
#include "Regex.hpp"
#include "utils.hpp"

Regex::Regex() {}

Regex::Regex(const std::string& expr) : expression(expr) {
    bool escape = false;
    std::size_t i = 0;
    std::size_t length = expr.size();
    unsigned nestingLevel = 0;

    std::deque<Composition> tokens;
    Composition comp;
    comp.pattern = PATTERN_CONTEXT_START;
    comp.special = true;
    tokens.push_back(std::move(comp));

    while (i < length) {
        bool special = false;
        bool skip = false;
        char c = expr[i];
        std::string pattern;
        pattern = c;
        if (!escape) {
            skip = true;
            switch (c) {
                case '*':
                    assert(tokens.size() > 0);
                    tokens.back().min = 0;
                    tokens.back().max = -1;
                    break;
                case '+':
                    assert(tokens.size() > 0);
                    tokens.back().min = 1;
                    tokens.back().max = -1;
                    break;
                case '?':
                    assert(tokens.size() > 0);
                    tokens.back().min = 0;
                    tokens.back().max = 1;
                    break;
                case '{': {
                    assert(tokens.size() > 0);
                    int start = -1;
                    int end = -1;
                    i++;
                    pattern.clear();
                    while (expr[i] != '}' && i < length) {
                        if (expr[i] == ',') {
                            start = stoi(pattern);
                            pattern.clear();
                        } else {
                            pattern += expr[i];                        
                        }
                        i++;
                    }
                    assert(i < length);
                    if (!pattern.empty()) {
                        end = stoi(pattern);                    
                    }
                    if (start < 0) {
                        start = end;
                    }
                    tokens.back().min = start;
                    tokens.back().max = end;
                    break;
                }
                case '|':
                    pattern = PATTERN_OR;
                    skip = false;
                    special = true;
                    break;
                case '(':
                    nestingLevel++;
                    pattern = PATTERN_CONTEXT_START;
                    skip = false;
                    special = true;
                    break;
                case ')':
                    assert(nestingLevel > 0);
                    nestingLevel--;
                    pattern = PATTERN_CONTEXT_END;
                    skip = false;
                    special = true;
                    break;
                case '[':
                    i++;
                    while (expr[i] != ']' && i < length) {
                        pattern += expr[i];
                        i++;
                    }
                    assert(i < length);
                    pattern += ']';
                    skip = false;
                    break;
                case '\\':
                    escape = true;
                    break;
                case '.':
                    pattern = PATTERN_WILDCARD;
                    skip = false;
                    break;
                default:
                    skip = false;
            }
        }

        if (!skip) {
            escape = false;
            Composition comp;
            comp.pattern = std::move(pattern);
            comp.nestingLevel = nestingLevel;
            comp.special = special;
            tokens.push_back(std::move(comp));
        }
        i++;
    }

    assert(nestingLevel == 0);

    Composition end;
    end.pattern = PATTERN_CONTEXT_END;
    end.special = true;
    tokens.push_back(std::move(end));

    // for (i = 0; i < tokens.size(); i++) {
    //     tokens[i].id = i;
    //     debug(tokens[i]);
    //     ECHO("");
    // }
    // assert(false);

    build(tokens);
}

void Regex::build(std::deque<Regex::Composition>& tokens) {
    // Expands tokes until [min,max] = [0,1] or [1,1] or [0,inf] for all of them
    std::deque<Composition> newList;
    while (!tokens.empty()) {
        Composition& token = tokens.front();
        // TODO: remove duplicated code
        if ((token.min != 0 || token.max != 1)
            && (token.min != 1 || token.max != 1)
            && !token.ready
            /*&& (token.min != 0 || token.max != -1)*/) {
            Composition newToken;
            newToken.pattern = token.pattern;
            newToken.min = (token.min == 0) ? 0 : token.min - 1;
            newToken.max = (token.max == -1) ? -1 : token.max - 1;
            if (token.min == 0 && token.max == -1) {
                newToken.ready = true;
            }
            newToken.nestingLevel = token.nestingLevel;
            newToken.special = token.special;
            newList.push_back(std::move(token));
            tokens.pop_front();
            tokens.push_front(std::move(newToken));
        } else {
            newList.push_back(std::move(token));
            tokens.pop_front();
        }
    }

    tokens.swap(newList);
    std::size_t i = 0;
    std::size_t size = tokens.size();
    // for (i = 0; i < tokens.size(); i++) {
    //     tokens[i].id = i;
    //     debug(tokens[i]);
    //     ECHO("");
    // }
    // assert(false);

    // Calculates all next references
    while (i < size - 1) {
        if (tokens[i].pattern == PATTERN_CONTEXT_START) {
            tokens[i].next.push_back(i + 1);
            unsigned level = tokens[i].nestingLevel;
            std::size_t j = i + 1;
            while (tokens[j].nestingLevel >= level && j < size) {
                if (tokens[j].nestingLevel == level && tokens[j].pattern == PATTERN_OR) {
                    tokens[i].next.push_back(j + 1);
                }
                j++;
            }
        } else if (tokens[i + 1].pattern != PATTERN_OR) {
            tokens[i].next = {i + 1};
        } else {
            std::size_t j = i + 1;
            int count = 0;
            // Will always find a match since the last token is a )
            while (count > 0 || tokens[j].pattern != PATTERN_CONTEXT_END) {
                if (tokens[j].pattern == PATTERN_CONTEXT_START) {
                    count++;
                } else if (tokens[j].pattern == PATTERN_CONTEXT_END) {
                    count--;
                }
                j++;
            }
            tokens[i].next = {j};
        }
        i++;
    }

    std::unordered_map<std::size_t, std::size_t> entityToState;
    // Creates all necessary states
    for (i = 0; i < size; i++) {
        if (tokens[i].pattern != PATTERN_OR) {
            entityToState[i] = stateList.size();
            stateList.push_back(State());
        }
    }
    acceptingState = entityToState[size - 1];

    i = 0;
    // Creates all transitions
    while (i < size) {
        Composition& token = tokens[i];
        if (token.pattern != PATTERN_OR) {
            for (std::size_t index : token.next) {
                std::size_t from = entityToState[i];
                std::size_t to = entityToState[index];
                if (token.special) {
                    stateList[from].spontaneous.insert(to);
                } else {
                    stateList[from].transitions[token.pattern] = to;
                    if (token.min == 0) {
                        stateList[from].spontaneous.insert(to);
                    }
                    if (token.max == -1 && token.ready) {
                        stateList[from].spontaneous.insert(entityToState[i - 1]);
                    }
                }
            }
        }
        i++;
    }

    for (i = 0; i < tokens.size(); i++) {
        tokens[i].id = i;
        debug(tokens[i]);
        ECHO("");
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
    TRACE(comp.min);
    TRACE(comp.max);
    TRACE(comp.nestingLevel);
    TRACE(comp.special);
    if (comp.next.size() > 0) {
        std::cout << "comp.next = ";
        for (std::size_t i = 0; i < comp.next.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << comp.next[i];
        }
        std::cout << std::endl;
    } else {
        TRACE_L("comp.next", "undefined");
    }
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
        } else if (pair.first.front() == '[' && pair.first.back() == '.') {
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
