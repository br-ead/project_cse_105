#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <set>
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
   /* if (!file) {
    cout << "Unable to open file: " << filename << endl;
    }
    if (file) {
        cout << "Opened file: " << filename << endl;
    } */
        while (getline(file, line)) {
            StateProps s;
            istringstream iss(line);
            string token;
            getline(iss, s.state, '-');
            getline(iss, token, '-');
            s.start = (token == "true");
            getline(iss, token, '-');
            s.finish = (token == "true");

        // Read routes
        while (getline(iss, token, '-')) {
            if (token == "x") {
                while (getline(iss, token, '-') && token != "y") {
                    if (token != "null") {
                        s.route_a.push_back(token);
                    }
                }
            } else if (token != "y" && token != "null") {
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

bool isFinalState(const string& state, const vector<StateProps>& dfa) {
    // Iterate over the DFA states
    for (const auto& dfaState : dfa) {
        // If the state is found and it's a final state, return true
        if (dfaState.state == state && dfaState.finish) {
            return true;
        }
    }
    // If the state is not found or it's not a final state, return false
    return false;
}

void identifyNewStates(vector<StateProps>& dfa, const vector<StateProps>& nfa) {
    vector<StateProps> newStates;
    set<string> existingStateNames; // To efficiently check for existing states

    // Populate existingStateNames with current DFA state names
    for (const auto& state : dfa) {
        existingStateNames.insert(state.state);
    }

    auto createStateName = [](const string& state1, const string& state2) -> string {
        if (state1 == state2) {
            return state1; // No need to create a composite if both states are the same
        }
        vector<string> parts = {state1, state2};
        sort(parts.begin(), parts.end()); // Sort to ensure consistent naming
        return "[" + parts[0] + "," + parts[1] + "]"; // Create composite name
    };

    // For each state in the DFA
    for (auto& dfaState : dfa) {
        // Handle transitions for route_a and route_b similarly, consider wrapping in a function for reuse
        auto handleRoute = [&](const vector<string>& route) {
            for (const auto& targetState : route) {
                string newStateName = createStateName(dfaState.state, targetState);

                // Check if the composite state already exists
                if (existingStateNames.find(newStateName) == existingStateNames.end()) {
                    StateProps newState;
                    newState.state = newStateName;
                    newState.start = false;
                    newState.finish = isFinalState(targetState, nfa); // Assuming isFinalState checks the NFA for finality
                    newStates.push_back(newState);
                    existingStateNames.insert(newStateName); // Add to existing state names to avoid duplicates
                }
            }
        };

        // Process transitions for route_a and route_b
        handleRoute(dfaState.route_a);
        handleRoute(dfaState.route_b);
    }

    // Add all new states to the DFA
    dfa.insert(dfa.end(), newStates.begin(), newStates.end());
}


bool representsState(const string& dfaState, const string& nfaState) {
    if (dfaState == nfaState) return true; // Direct match

    // Split DFA state into components and check if any component matches the NFA state
    vector<string> components;
    size_t start = 0, end = 0;
    while ((end = dfaState.find(',', start)) != string::npos) {
        components.push_back(dfaState.substr(start, end - start));
        start = end + 1;
    }
    components.push_back(dfaState.substr(start)); // Add the last component

    return find(components.begin(), components.end(), nfaState) != components.end();
}

void determineTransitions(vector<StateProps>& dfa, const vector<StateProps>& nfa) {
    // For each state in the DFA
    for (auto& dfaState : dfa) {
        // For each state in the NFA
        for (const auto& nfaState : nfa) {
            // If the DFA state contains the NFA state
            if (representsState(dfaState.state, nfaState.state)) {
                // Add the transitions of the NFA state to the DFA state
                dfaState.route_a.insert(nfaState.route_a.begin(), nfaState.route_a.end());
                dfaState.route_b.insert(nfaState.route_b.begin(), nfaState.route_b.end());
            }
        }
    }
}

vector<StateProps> initializeDFA(const vector<StateProps>& nfa, const string& initialState) {
    vector<StateProps> dfa;

    // Find the start state in the NFA
    for (const auto& state : nfa) {
        if (state.state == initialState) {
            dfa.push_back(state);
            break;
        }
    }

    return dfa;
}

vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;
    set<string> processedStates;

    // Step 1: Determine initial state of DFA
    string initialState = findInitialState(nfa);
    dfa = initializeDFA(nfa, initialState);

    // Step 2: State Expansion
    queue<string> stateQueue;
    stateQueue.push(initialState);
    while (!stateQueue.empty()) {
        string currentState = stateQueue.front();
        stateQueue.pop();

        // Step 3: Determine transitions on each input symbol
        determineTransitions(dfa, nfa);

        // Step 4: Identify new DFA states
        identifyNewStates(dfa, nfa);
    }

    return dfa;
}


void printStates(const vector<StateProps>& states) {
    for (const auto& state : states) {
        cout << state.state << " is " << (state.start ? "" : "not ") << "a start state. ";
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
    //cout << "Read " << nfa.size() << " states from the file." << endl;
    //cout << "This is the NFA" << endl;
    // printStates(nfa);
    vector<StateProps> dfa=convertNFAtoDFA(nfa);
    //cout << "This is the DFA" << endl;
    //printStates(dfa);

    return 0;
}
