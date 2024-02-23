#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
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

string findInitialState(const vector<StateProps>& nfa) {
    for (const auto& stateNFA : nfa) {
        if (stateNFA.start) {
            return stateNFA.state;
        }
    }
    return "";
}

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
// prompt -- Using findInitialState function generate a way to initialize a DFA for my NFA to DFA procedure.

bool isFinalState(const string& compositeState, const vector<StateProps>& dfa) {
    // Split the composite state into individual states
    stringstream ss(compositeState);
    string state;
    while (getline(ss, state, '/')) {
        // cout << "test" << endl;
        for (const auto& dfaState : dfa) {
            //cout << "test2" << endl;
            //cout << dfaState.state;
            if (dfaState.state == state && dfaState.finish) {
                // cout << dfaState.state << " lol " << endl;
                return true; // Return true if any of the states is a final state
            }
        }
    }
    return false;
}
// prompt -- Some of my values are composites. They are separated by a delimiter, if any of them, however, are a final
// state I want the state to be a final state.

string convertSetToStateName(const set<string>& stateSet) {
    string stateName;
    for (const auto& state : stateSet) {
        if (!stateName.empty()) stateName += "/";
        stateName += state;
    }
    return stateName.empty() ? "null" : stateName;
}
// prompt -- Help me generate a function that will generate a new stateName for any potential composite states that
// I create

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
// prompt -- Hello, I want to take in two values, route_a and route_b and computes the NextState based off of the adjustments 
// made in updateTransitionTable

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
// prompt - hello I do not want states that are similar, i.e. q0/q1 and q1/0 can you make a way for me to only add values if they are 
// not currently present?

StateProps createNewState(const string& stateName, bool isFinal, const vector<string>& routeA, const vector<string>& routeB) {
    StateProps newState;
    newState.state = stateName;
    newState.start = false; // New states are never start states :D
    newState.finish = isFinal;
    newState.route_a = routeA;
    newState.route_b=routeB;
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
            break; 
        }
    }
}
// this will be called after every iteration of the while loop in nfatodfa. 

bool needsDeathState(const vector<StateProps>& dfa) {
    for (const auto& state : dfa) {
        if (state.route_a.empty() || state.route_b.empty()) {
            return true;
        }
    }
    return false;
}

// create sink

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
    vector<StateProps> dfa;
    queue<string> newStates;
    newStates.push(initialState);

    // A map to keep track of transitions for composite states
    map<string, vector<string>> transitionA, transitionB;

    while (!newStates.empty()) {
        string currentState = newStates.front();
        newStates.pop();

        // Initialize transition vectors for the current composite state
        vector<string> currentStateTransA, currentStateTransB;

        for (char input : {'a', 'b'}) {
            set<string> nextStateSet = computeNextState(currentState, input, nfa);
            string nextState = convertSetToStateName(nextStateSet);

            vector<string> aggregatedRouteA, aggregatedRouteB;
            for (const string& state : nextStateSet) {
                auto it = find_if(nfa.begin(), nfa.end(), [&](const StateProps& sp) { return sp.state == state; });
                if (it != nfa.end()) {
                    if (input == 'a') {
                        aggregatedRouteA.insert(aggregatedRouteA.end(), it->route_a.begin(), it->route_a.end());
                    } else {
                        aggregatedRouteB.insert(aggregatedRouteB.end(), it->route_b.begin(), it->route_b.end());
                    }
                }
            }

            // Ensure transitions are unique
            sort(aggregatedRouteA.begin(), aggregatedRouteA.end());
            aggregatedRouteA.erase(unique(aggregatedRouteA.begin(), aggregatedRouteA.end()), aggregatedRouteA.end());
            sort(aggregatedRouteB.begin(), aggregatedRouteB.end());
            aggregatedRouteB.erase(unique(aggregatedRouteB.begin(), aggregatedRouteB.end()), aggregatedRouteB.end());

            // Update currentStateTransA or currentStateTransB based on input
            if (input == 'a') currentStateTransA = aggregatedRouteA;
            else currentStateTransB = aggregatedRouteB;

            if (!nextState.empty() && !isStateInDFA(nextState, dfa)) {
                bool finalState = isFinalState(nextState, nfa); // Make sure isFinalState checks correctly for composite states
                StateProps newState = createNewState(nextState, finalState, currentStateTransA, currentStateTransB);
                dfa.push_back(newState);
                newStates.push(nextState);
            }
        }

        // Directly updating the DFA state with aggregated transitions
        auto dfaIt = find_if(dfa.begin(), dfa.end(), [&](const StateProps& sp) { return sp.state == currentState; });
        if (dfaIt != dfa.end()) {
            dfaIt->route_a = currentStateTransA;
            dfaIt->route_b = currentStateTransB;
        } else {
            // This handles the initial state specifically if it hasn't been added yet
            bool isFinal = isFinalState(currentState, nfa);
            StateProps initialState = createNewState(currentState, isFinal, currentStateTransA, currentStateTransB);
            dfa.push_back(initialState);
        }
    }

    addDeathStateIfNeeded(dfa);
    printStates(dfa);
}

/*
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
            if (/*!nextStateSet.empty() &&*/ !isStateInDFA(nextState, dfa)) {
                bool finalState = false;
                for (const string& state : nextStateSet) {
                    if (isFinalState(state, dfa)) {
                        finalState = true;
                        break;
                    }
                }
                StateProps newState = createNewState(nextState, finalState);
                dfa.push_back(newState);
                newStates.push(nextState);
            }

            // Update the transition table only if nextState is not "null"
            if (nextState != "null") {
                updateTransitionTable(currentState, input, nextState, dfa);
            }
        }
    }

    // Filter out states with no valid transitions before printing
    dfa.erase(remove_if(dfa.begin(), dfa.end(), [](const StateProps& state) {
        return state.route_a.empty() && state.route_b.empty();
    }), dfa.end()); 
    addDeathStateIfNeeded(dfa);
    printStates(dfa);
} */
// using all of my functions generate a nfatodfa function that takes in an nfa and makes a resulting dfa based off of subset construction. 
// Also print it using my previous function

    
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
