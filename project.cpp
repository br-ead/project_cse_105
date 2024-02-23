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
    while (getline(file, line)) {
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

bool isFinalState(const string& state, const vector<StateProps>& dfa) {
    for (const auto& dfaState : dfa) {
        if (dfaState.state == state && dfaState.finish) {
            return true;
        }
    }
    return false;
}

vector<StateProps> initializeDFA(const vector<StateProps>& nfa, const string& initialState) {
    vector<StateProps> dfa;
    for (const auto& state : nfa) {
        if (state.state == initialState) {
            dfa.push_back(state);
            break;
        }
    }
    return dfa;
}

set<string> findClosure(const vector<StateProps>& nfa, const set<string>& states) {
    set<string> closure = states;
    // Example logic for finding closure; adjust based on your NFA structure
    // This part can be expanded based on how your NFA handles epsilon transitions or similar
    return closure;
}

// Converts a set of NFA states into a DFA state (string representation)
string convertSetToStateName(const set<string>& stateSet) {
    string stateName;
    for (const auto& state : stateSet) {
        if (!stateName.empty()) stateName += ",";
        stateName += state;
    }
    return stateName;
}

// Main conversion function
vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    // Step 3: Initialize DFA with the initial state
    vector<StateProps> dfa;
    string initialState = findInitialState(nfa);
    set<string> initialStates = {initialState};
    queue<set<string>> stateQueue;
    stateQueue.push(initialStates);
    set<string> visitedStates; // Keep track of visited states to avoid processing a state more than once

    while (!stateQueue.empty()) {
        auto currentStates = stateQueue.front();
        stateQueue.pop();

        StateProps newState;
        newState.state = convertSetToStateName(currentStates);
        if (visitedStates.find(newState.state) != visitedStates.end()) {
            continue; // Skip if this set of states has already been processed
        }
        visitedStates.insert(newState.state);

        // Initialize new state properties
        newState.start = (newState.state == initialState);
        newState.finish = any_of(currentStates.begin(), currentStates.end(), [&nfa](const string& state) {
            return isFinalState(state, nfa); // Assuming isFinalState can be used or adapted for this purpose
        });

        // Step 4 and 5: Determine transitions for the new state
        set<string> nextStatesA, nextStatesB;
        for (const auto& state : currentStates) {
            // Find corresponding NFA state and its transitions
            auto it = find_if(nfa.begin(), nfa.end(), [&state](const StateProps& s) {
                return s.state == state;
            });
            if (it != nfa.end()) {
                nextStatesA.insert(it->route_a.begin(), it->route_a.end());
                nextStatesB.insert(it->route_b.begin(), it->route_b.end());
            }
        }

        // Convert sets of next states into state names and queue them for processing
        if (!nextStatesA.empty()) {
            newState.route_a.insert(convertSetToStateName(nextStatesA));
            if (visitedStates.find(convertSetToStateName(nextStatesA)) == visitedStates.end()) {
                stateQueue.push(nextStatesA);
            }
        }
        if (!nextStatesB.empty()) {
            newState.route_b.insert(convertSetToStateName(nextStatesB));
            if (visitedStates.find(convertSetToStateName(nextStatesB)) == visitedStates.end()) {
                stateQueue.push(nextStatesB);
            }
        }

        // Step 6: Add the new state to DFA
        dfa.push_back(newState);
    }

    // Step 7: Return the constructed DFA
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
