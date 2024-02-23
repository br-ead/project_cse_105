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
            else if (token != "y" && token != "null") {
                s.route_b.push_back(token);
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
// We use this in conjunction with initializeDFA to generate our starter DFA.

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
    for (const auto& state : composite) {
        auto it = find_if(nfa.begin(), nfa.end(), [&](const StateProps& sp) { return sp.state == state; });
        if (it != nfa.end() && it->finish) return true;
    }
    return false;
}

set<string> computeNextState(const string& currentState, char input, const vector<StateProps>& nfa) {
    set<string> nextStateSet;
    stringstream ss(currentState);
    string state;
    while (getline(ss, state, '/')) {
        if (state.empty()) continue; // Skip empty states
        for (const auto& nfaState : nfa) {
            if (nfaState.state == state) {
                const vector<string>& routes = (input == 'a') ? nfaState.route_a : nfaState.route_b;
                for (const auto& route : routes) {
                    if (!route.empty()) {
                        nextStateSet.insert(route);
                    }
                }
            }
        }
    }
    return nextStateSet;
}


bool isStateInDFA(const string& state, const vector<StateProps>& dfa) {
    return find_if(dfa.begin(), dfa.end(), [&](const StateProps& sp) { return sp.state == state; }) != dfa.end();
}

StateProps createNewState(const string& stateName, bool isFinal) {
    StateProps newState;
    newState.state = stateName;
    newState.start = false; // New states are never start states
    newState.finish = isFinal;
    return newState;
}

void updateTransitionTable(const string& currentState, char input, const string& nextState, vector<StateProps>& dfa) {
    auto it = find_if(dfa.begin(), dfa.end(), [&](const StateProps& sp) { return sp.state == currentState; });
    if (it != dfa.end()) {
        if (input == 'a') {
            it->route_a.clear();
            it->route_a.push_back(nextState);
        } else if (input == 'b') {
            it->route_b.clear();
            it->route_b.push_back(nextState);
        }
    }
}

void convertNFAtoDFA(const vector<StateProps>& nfa) {
    // Find the initial state of the NFA
    string initialState = findInitialState(nfa);

    // Initialize the DFA with the initial state
    vector<StateProps> dfa = initializeDFA(nfa, initialState);

    // Create a queue to hold the new states of the DFA
    queue<string> newStates;
    newStates.push(initialState);

    // Process each new state
    while (!newStates.empty()) {
        string currentState = newStates.front();
        newStates.pop();

        // For each input symbol...
        for (char input : {'a', 'b'}) { // assuming 'a' and 'b' are the input symbols
            // Compute the new state for the current input
            set<string> nextStateSet = computeNextState(currentState, input, nfa);

            // Convert the set of states to a state name
            string nextState = convertSetToStateName(nextStateSet);

            // If the new state is not already in the DFA...
            if (!isStateInDFA(nextState, dfa)) {
                // Add the new state to the DFA
                dfa.push_back(createNewState(nextState, isCompositeFinal(nextStateSet, nfa)));

                // Add the new state to the queue
                newStates.push(nextState);
            }

            // Update the transition table for the current state and input
            updateTransitionTable(currentState, input, nextState, dfa);
        }
    }

    printStates(dfa);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string filename = argv[1];
    
    vector<StateProps> nfa = readStatesFromFile(filename);
    // cout << "Read " << nfa.size() << " states from the file." << endl;
    //cout << "This is the NFA" << endl;
    //printStates(nfa);
    //vector<StateProps> dfa= convertNFAtoDFA(nfa);
    //vector<StateProps> dfa=convertNFAtoDFA(nfa);
    cout << "This is the DFA" << endl;
    // printStates(dfa);

    convertNFAtoDFA(nfa);

    return 0;
}
