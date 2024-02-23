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
    while (getline(file, line)) {
        istringstream iss(line);
        StateProps state;
        string start, finish, symbol, target;
        getline(iss, state.state, '-');
        getline(iss, start, '-');
        getline(iss, finish, '-');
        state.start = (start == "true");
        state.finish = (finish == "true");

        // Read transitions for 'x' (symbol 'a')
        getline(iss, symbol, '-'); // Should always be 'x'
        while (getline(iss, target, '-') && target != "y") {
            if (target != "null") state.route_a.push_back(target);
        }

        // Read transitions for 'y' (symbol 'b'), assuming 'y' has been consumed
        while (getline(iss, target, '-') && !iss.eof()) {
            if (target != "null") state.route_b.push_back(target);
        }

        states.push_back(state);
    }
    return states;
}

// Helper function to check if any NFA state in the set is a final state
bool isFinalState(const vector<StateProps>& nfa, const set<string>& stateSet) {
    for (const auto& s : stateSet) {
        auto it = find_if(nfa.begin(), nfa.end(), [&s](const StateProps& sp) { return sp.state == s; });
        if (it != nfa.end() && it->finish) return true;
    }
    return false;
}

set<string> getReachableStates(const vector<StateProps>& nfa, const set<string>& states, char symbol) {
    set<string> reachableStates;
    for (const auto& currentState : states) {
        for (const auto& nfaState : nfa) {
            if (currentState == nfaState.state) {
                const vector<string>& transitions = (symbol == 'a') ? nfaState.route_a : nfaState.route_b;
                reachableStates.insert(transitions.begin(), transitions.end());
            }
        }
    }
    return reachableStates;
}

// Helper function to join set elements into a string
string joinSet(const set<string>& stateSet) {
    string result;
    for (const auto& s : stateSet) {
        if (!result.empty()) result += "_";
        result += s;
    }
    return result;
}

vector<StateProps> nfaToDFA(const vector<StateProps>& nfa) {
    map<set<string>, StateProps> dfa; // Holds the DFA states
    map<set<string>, map<char, set<string>>> transitions; // Transition function for DFA
    queue<set<string>> processingQueue; // Queue for processing states

    // Identify the initial state and add it to the queue
    set<string> initialStateSet;
    for (const auto& s : nfa) {
        if (s.start) {
            initialStateSet.insert(s.state);
            break; // Assuming there's only one start state
        }
    }
    processingQueue.push(initialStateSet);
    transitions[initialStateSet]; // Initialize transitions for the initial state

    while (!processingQueue.empty()) {
        set<string> currentSet = processingQueue.front();
        processingQueue.pop();

        // Generate a unique name for the current DFA state
        string dfaStateName = joinSet(currentSet);

        // Check if this DFA state is already processed
        if (dfa.find(currentSet) == dfa.end()) {
            StateProps newState;
            newState.state = dfaStateName;
            newState.start = (currentSet == initialStateSet); // Mark as start if it matches the initial NFA state
            newState.finish = isFinalState(nfa, currentSet); // Determine if this is a final state
            
            // Initialize empty transitions
            newState.route_a = {};
            newState.route_b = {};

            dfa[currentSet] = newState;
        }

        // Process transitions for 'a' and 'b'
        for (char symbol : {'a', 'b'}) {
            set<string> newStateSet = getReachableStates(nfa, currentSet, symbol);
            if (!newStateSet.empty()) {
                transitions[currentSet][symbol] = newStateSet;
                if (dfa.find(newStateSet) == dfa.end()) {
                    processingQueue.push(newStateSet); // Process new state
                }
            }
        }
    }

    // Update the DFA transitions based on transitions map
    for (auto& [stateSet, stateProps] : dfa) {
        for (char symbol : {'a', 'b'}) {
            if (transitions[stateSet].find(symbol) != transitions[stateSet].end()) {
                set<string> targetSet = transitions[stateSet][symbol];
                string targetStateName = joinSet(targetSet);
                if (symbol == 'a') stateProps.route_a.push_back(targetStateName);
                else stateProps.route_b.push_back(targetStateName);
            }
        }
    }

    // Convert the map to a vector for the final DFA
    vector<StateProps> dfaVector;
    for (const auto& [_, stateProps] : dfa) {
        dfaVector.push_back(stateProps);
    }

    return dfaVector;
}

void printDFA(const vector<StateProps>& dfa) {
    cout << "DFA States and Transitions:\n";
    for (const auto& state : dfa) {
        cout << "State: " << state.state;
        if (state.start) cout << " (Start State)";
        if (state.finish) cout << " (Final State)";
        cout << "\n";

        // Print transitions for 'a'
        if (!state.route_a.empty()) {
            cout << "  On 'a' -> ";
            for (const auto& dest : state.route_a) {
                cout << dest << " ";
            }
            cout << "\n";
        } else {
            cout << "  On 'a' -> No Transition\n";
        }

        // Print transitions for 'b'
        if (!state.route_b.empty()) {
            cout << "  On 'b' -> ";
            for (const auto& dest : state.route_b) {
                cout << dest << " ";
            }
            cout << "\n";
        } else {
            cout << "  On 'b' -> No Transition\n";
        }
        cout << "\n"; // Add an extra newline for spacing
    }
}

int main() {
    // Assuming `filename` is the path to your input file with NFA definitions
    string filename = "your_nfa_file.txt";
    vector<StateProps> nfa = readStatesFromFile(filename);

    // Convert NFA to DFA
    vector<StateProps> dfa = nfaToDFA(nfa);

    // Print the resulting DFA
    printDFA(dfa);

    return 0;
}
