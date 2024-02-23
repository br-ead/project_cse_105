#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <set>
#include <map>
using namespace std;

struct StateProps {
    string state;
    bool start;
    bool finish;
    vector<string> route_a;
    vector<string> route_b;
};

vector<StateProps> readStatesFromFile(const string& filename) {
    vector<StateProps> states;
    ifstream file(filename);
    string line;
    while (getline(file, line)) { // my input uses the delimiter -
        StateProps s;
        istringstream iss(line);
        string token;
        getline(iss, s.state, '-');
        getline(iss, token, '-');
        s.start = (token == "true");
        getline(iss, token, '-');
        s.finish = (token == "true");
        while (getline(iss, token, '-')) {
            if (token == "x") {
                while (getline(iss, token, '-') && token != "y") {
                    if (token != "null") {
                        s.route_a.push_back(token);
                    }
                }
            }
            else if (token != "y" && token != "null") {
                s.route_b.push_back(token);
            }
            }
    states.push_back(s);
    }
    return states;
}
// Iterate through file with contents like state-true-finish- etc.

string findInitialState(const vector<StateProps>& nfa) {
    for (const auto& stateNFA : nfa) {
        if (stateNFA.start) {
            return stateNFA.state;
        }
    }
    return "";
}
// We use this with initializeDFA to generate our starter DFA.

vector<StateProps> initializeDFA(const vector<StateProps>& nfa, const string& initialState) {
    vector<StateProps> dfa;
    for (const auto& newState : nfa) {
        if (newState.state == initialState) {
            dfa.push_back(newState);
            break;
        }
    }
    return dfa;
}
// We use this to make our starter DFA.

bool isFinalState(const string& compositeState, const vector<StateProps>& dfa) {
    // Split the composite state into individual states
    stringstream ss(compositeState);
    string state;
    while (getline(ss, state, '/')) { // Assuming '/' is the delimiter
        // Check each state in the composite state
        for (const auto& dfaState : dfa) {
            if (dfaState.state == state && dfaState.finish) {
                return true; // Return true if any of the states is a final state
            }
        }
    }
    return false; // Return false if none of the states are final states
}

// Chat gpt work ^, basically iterates through the string, if either qi or qj is true then it is a finish state.

string convertSetToStateName(const set<string>& stateSet) {
    string stateName;
    for (const auto& state : stateSet) {
        if (!stateName.empty()) stateName += "/";
        stateName += state;
    }
    return stateName.empty() ? "null" : stateName;
}
// Implemented by chat GPT ^

void printStates(const vector<StateProps>& states) {
    for (const auto& stateEntry : states) {
        cout << stateEntry.state << " is " << (stateEntry.start ? "" : "not ") << "a start state. ";
        cout << "It is " << (stateEntry.finish ? "" : "not ") << "a finish state. When the input is a, it will route to ";
        
        if (stateEntry.route_a.empty()) {
            cout << "nothing";
        } else {
            for (size_t i = 0; i < stateEntry.route_a.size(); ++i) {
                cout << stateEntry.route_a[i];
                if (i != stateEntry.route_a.size() - 1) {
                    cout << ",";
                }
            }
        }
        cout << " and when the input is b, it will route to ";
        if (stateEntry.route_b.empty()) {
            cout << "nothing";
        } else {
            for (size_t i = 0; i < stateEntry.route_b.size(); ++i) {
                cout << stateEntry.route_b[i];
                if (i != stateEntry.route_b.size() - 1) {
                    cout << ",";
                }
            }
        }
        cout << endl;
    }
}
// self explanatory

bool isCompositeFinal(const set<string>& composite, const vector<StateProps>& nfa) {
    for (const string& compState : composite) {
        for (size_t i = 0; i < nfa.size(); ++i) {
            if (nfa[i].state == compState && nfa[i].finish) {
                return true; // If any of the composite states is a final state in the NFA, return true
            }
        }
    }
    return false; // If none of the composite states are final states, return false
}

set<string> computeNextState(const string& currentState, char input, const vector<StateProps>& nfa) {
    set<string> nextStateSet;
    stringstream ss(currentState);
    string state;
    while (getline(ss, state, '/')) {
        for (const auto& nfaState : nfa) {
            if (nfaState.state == state) {
                const vector<string>& routes = (input == 'a') ? nfaState.route_a : nfaState.route_b;
                for (const auto& route : routes) {
                    if (!route.empty() && route != "null") {
                        nextStateSet.insert(route);
                    }
                }
            }
        }
    }
    return nextStateSet;
}


bool isStateInDFA(const string& state, const vector<StateProps>& dfa) {
    // Check for exact match
    if (find_if(dfa.begin(), dfa.end(), [&](const StateProps& sp) { return sp.state == state; }) != dfa.end()) {
        return true;
    }

    // Check for permutations of composite states
    size_t pos = state.find('/');
    if (pos != string::npos) {
        string state1 = state.substr(0, pos);
        string state2 = state.substr(pos + 1);
        string statePermutation = state2 + '/' + state1;
        return (find_if(dfa.begin(), dfa.end(), [&](const StateProps& sp) { return sp.state == statePermutation; }) != dfa.end());
    }

    return false;
}

StateProps createNewState(const string& stateName, bool isFinal) {
    StateProps newState;
    newState.state = stateName;
    newState.start = false; // New states are never start states
    newState.finish = isFinal;
    return newState;
}

void updateTransitionTable(const string& currentState, char input, const string& nextState, vector<StateProps>& dfa) {
    for (size_t i = 0; i < dfa.size(); ++i) {
        if (dfa[i].state == currentState) {
            if (input == 'a') {
                dfa[i].route_a.clear();
                dfa[i].route_a.push_back(nextState);
            } else if (input == 'b') {
                dfa[i].route_b.clear();
                dfa[i].route_b.push_back(nextState);
            }
            break; // Assuming state names are unique, we can exit the loop once the state is found and updated
        }
    }
}

bool needsDeathState(const vector<StateProps>& dfa) {
    for (const auto& state : dfa) {
        if (state.route_a.empty() || state.route_b.empty()) {
            return true;
        }
    }
    return false;
}

void addDeathStateIfNeeded(vector<StateProps>& dfa) {
    bool deathStateNeeded = false;

    // First, determine if a "death" state is needed by checking existing transitions.
    for (const auto& state : dfa) {
        if (state.route_a.empty() || state.route_b.empty()) {
            deathStateNeeded = true;
            break;
        }
    }

    if (!deathStateNeeded) {
        return; // No "death" state needed, return early.
    }

    // Create and add the "death" state to DFA.
    StateProps deathState;
    deathState.state = "death";
    deathState.start = false;
    deathState.finish = false;
    deathState.route_a.push_back("death");
    deathState.route_b.push_back("death");
    dfa.push_back(deathState);

    // Correctly update states with missing transitions to include "death".
    for (auto& state : dfa) {
        if (state.route_a.empty()) {
            state.route_a.push_back("death");
        }
        if (state.route_b.empty()) {
            state.route_b.push_back("death");
        }
    }
}



void convertNFAtoDFA(const vector<StateProps>& nfa) {
    string initialState = findInitialState(nfa);
    vector<StateProps> dfa = initializeDFA(nfa, initialState);
    queue<string> newStates;
    newStates.push(initialState);

    while (!newStates.empty()) {
        string currentState = newStates.front();
        newStates.pop();

        for (char input : {'a', 'b'}) {
            set<string> nextStateSet = computeNextState(currentState, input, nfa);
            string nextState = convertSetToStateName(nextStateSet);

            // Check if nextState is meaningful before adding to DFA
            if (!nextStateSet.empty() && !isStateInDFA(nextState, dfa)) {
                StateProps newState = createNewState(nextState, isCompositeFinal(nextStateSet, nfa));
                dfa.push_back(newState);
                newStates.push(nextState);
            }

            // Update the transition table only if nextState is not "null"
            updateTransitionTable(currentState, input, nextState, dfa);
        }
        
    }

    // Filter out states with no valid transitions before printing
    dfa.erase(remove_if(dfa.begin(), dfa.end(), [](const StateProps& state) {
        return state.route_a.empty() && state.route_b.empty();
    }), dfa.end());
    addDeathStateIfNeeded(dfa);
    printStates(dfa);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string filename = argv[1];
    
    vector<StateProps> nfa = readStatesFromFile(filename);
    //cout << "Read " << nfa.size() << " states from the file." << endl;
    cout << "This is the NFA" << endl;
    printStates(nfa);
    cout << "This is the DFA" << endl;
    convertNFAtoDFA(nfa);

    return 0;
}
