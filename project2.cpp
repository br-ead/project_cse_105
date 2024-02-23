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

// Define StateProps struct to represent states and transitions
struct StateProps {
    string state;
    bool start;
    bool finish;
    vector<string> route_a;
    vector<string> route_b;
};

// Function to read states from file
vector<StateProps> readStatesFromFile(const string& filename) {
    vector<StateProps> states;
    ifstream file(filename);
    string line;
    while (getline(file, line)) { // Input uses the delimiter -
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

// Function to find the initial state
string findInitialState(const vector<StateProps>& nfa) {
    for (const auto& stateNFA : nfa) {
        if (stateNFA.start) {
            return stateNFA.state;
        }
    }
    return "";
}

// Function to initialize DFA with the initial state
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

// Function to check if a composite state is final
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

// Function to compute transitions and convert NFA to DFA
vector<StateProps> convertNFAToDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa; // DFA states
    set<set<string>> dfaStates; // Track visited DFA states
    queue<set<string>> stateQueue; // Queue for states to process

    // Initialize DFA with initial state {q0}
    string initialState = findInitialState(nfa);
    dfa = initializeDFA(nfa, initialState);
    dfaStates.insert({initialState});
    stateQueue.push({initialState});

    // Process states until the queue is empty
    while (!stateQueue.empty()) {
        set<string> currentState = stateQueue.front();
        stateQueue.pop();

        // Iterate over input symbols
        for (char input : {'a', 'b'}) {
            set<string> newState; // State to transition to for the current input

            // Compute the new state by following transitions for each state in the current composite state
            for (const string& state : currentState) {
                auto it = find_if(nfa.begin(), nfa.end(), [&](const StateProps& sp) { return sp.state == state; });
                if (it != nfa.end()) {
                    const vector<string>& routes = (input == 'a') ? it->route_a : it->route_b;
                    for (const auto& route : routes) {
                        if (!route.empty()) {
                            newState.insert(route);
                        }
                    }
                }
            }

            // Check if the new state is already in the DFA states
            auto it = find(dfaStates.begin(), dfaStates.end(), newState);
            if (it == dfaStates.end()) {
                // Add the new state to DFA states and enqueue it for further processing
                dfaStates.insert(newState);
                stateQueue.push(newState);
                dfa.push_back({convertSetToStateName(newState), false, isFinalState(convertSetToStateName(newState), dfa), {}, {}}); // Create new DFA state
                it = dfaStates.find(newState);
            }

            // Update DFA transitions
            if (!newState.empty()) {
                updateTransitionTable(convertSetToStateName(currentState), input, convertSetToStateName(newState), dfa);
            }
        }
    }

    return dfa;
}

// Function to update transition table
void updateTransitionTable(const string& currentState, char input, const string& nextState, vector<StateProps>& dfa) {
    for (auto& state : dfa) {
        if (state.state == currentState) {
            if (input == 'a') {
                state.route_a.clear();
                state.route_a.push_back(nextState);
            } else if (input == 'b') {
                state.route_b.clear();
                state.route_b.push_back(nextState);
            }
            break;
        }
    }
}

// Function to convert set of states to state name
string convertSetToStateName(const set<string>& stateSet) {
    string stateName;
    for (const auto& state : stateSet) {
        if (!stateName.empty()) stateName += "/";
        stateName += state;
    }
    return stateName.empty() ? "null" : stateName;
}

// Function to print DFA
void printDFA(const vector<StateProps>& dfa) {
    for (const auto& state : dfa) {
        cout << "State: " << state.state << " is " << (state.start ? "" : "not ") << "a start state. ";
        cout << "It is " << (state.finish ? "" : "not ") << "a finish state. When the input is a, it will route to ";
        if (state.route_a.empty()) {
            cout << "nothing";
        } else {
            for (size_t i = 0; i < state.route_a.size(); ++i) {
                cout << state.route_a[i];
                if (i != state.route_a.size() - 1) {
                    cout << ",";
                }
            }
        }
        cout << " and when the input is b, it will route to ";
        if (state.route_b.empty()) {
            cout << "nothing";
        } else {
            for (size_t i = 0; i < state.route_b.size(); ++i) {
                cout << state.route_b[i];
                if (i != state.route_b.size() - 1) {
                    cout << ",";
                }
            }
        }
        cout << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string filename = argv[1];
    
    vector<StateProps> nfa = readStatesFromFile(filename);
    vector<StateProps> dfa = convertNFAToDFA(nfa);

    cout << "This is the DFA" << endl;
    printDFA(dfa);

    return 0;
}
