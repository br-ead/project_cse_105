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
// Function to combine NFA states based on specific criteria for use in NFA to DFA conversion
void combineNFAStatesForDFA(vector<StateProps>& nfa) {
    // Temporary container for newly created composite states
    vector<StateProps> compositeStates;

    // Iterate through NFA to find states to combine
    for (const auto& state : nfa) {
        // Check each route_a for potential combination
        for (const auto& route : state.route_a) {
            // Skip null routes or self-transitions
            if (route == "null" || route == state.state) continue;

            // Attempt to find the target state specified in route_a
            const auto targetIt = find_if(nfa.begin(), nfa.end(), [&](const StateProps& sp) {
                return sp.state == route;
            });

            // If target state is found and it's not itself, proceed to combine
            if (targetIt != nfa.end()) {
                StateProps compositeState;
                set<string> combinedRoutesA, combinedRoutesB;

                // Aggregate transitions from both states, avoiding duplicates
                combinedRoutesA.insert(state.route_a.begin(), state.route_a.end());
                combinedRoutesA.insert(targetIt->route_a.begin(), targetIt->route_a.end());

                combinedRoutesB.insert(state.route_b.begin(), state.route_b.end());
                combinedRoutesB.insert(targetIt->route_b.begin(), targetIt->route_b.end());

                // Convert sets to vector and assign to composite state
                compositeState.route_a = vector<string>(combinedRoutesA.begin(), combinedRoutesA.end());
                compositeState.route_b = vector<string>(combinedRoutesB.begin(), combinedRoutesB.end());

                // Composite state name and attributes
                compositeState.state = state.state + "/" + route; // Custom logic for naming
                compositeState.start = false; // Composite states derived here are typically not start states
                compositeState.finish = state.finish || targetIt->finish; // Finish if either state is a finish state

                // Add the composite state to the temporary container
                compositeStates.push_back(compositeState);
            }
        }
    }

    // Append the composite states to the NFA
    nfa.insert(nfa.end(), compositeStates.begin(), compositeStates.end());
}

vector<StateProps> convertNFAtoDFA(vector<StateProps> nfa) {
    // Prepare the NFA by combining states as needed.
    combineNFAStatesForDFA(nfa); // This modifies the `nfa` directly.

    vector<StateProps> dfa; // This will store the final DFA states.
    map<set<string>, string> seenStates; // Maps each unique set of NFA states to a DFA state name.
    queue<set<string>> toProcess; // Queue to manage sets of NFA states that need to be processed.

    // Initialize with the initial state set.
    set<string> initialStateSet = {findInitialState(nfa)};
    toProcess.push(initialStateSet);
    seenStates[initialStateSet] = convertSetToStateName(initialStateSet);

    while (!toProcess.empty()) {
        set<string> currentSet = toProcess.front();
        toProcess.pop();
        StateProps newState;
        newState.state = seenStates[currentSet]; // Set the state name based on the seenStates mapping.

        // Determine if the new state is a start or finish state.
        newState.start = (currentSet.find(findInitialState(nfa)) != currentSet.end());
        newState.finish = isCompositeFinal(currentSet, nfa);

        // Transition results mapping for route_a and route_b.
        map<char, set<string>> transitionResults;

        // Collect transitions from current set of NFA states.
        for (const auto& stateName : currentSet) {
            auto it = find_if(nfa.begin(), nfa.end(), [&](const StateProps& sp) { return sp.state == stateName; });
            if (it != nfa.end()) {
                // Process transitions for route_a and route_b.
                for (const auto& dest : it->route_a) {
                    if (dest != "null") transitionResults['a'].insert(dest);
                }
                for (const auto& dest : it->route_b) {
                    if (dest != "null") transitionResults['b'].insert(dest);
                }
            }
        }

        // Process transition results to create new DFA states.
        for (const auto& [inputChar, resultSet] : transitionResults) {
            if (resultSet.empty()) continue;

            if (seenStates.find(resultSet) == seenStates.end()) {
                string newStateName = convertSetToStateName(resultSet);
                seenStates[resultSet] = newStateName;
                toProcess.push(resultSet);
            }

            // Add transitions to the DFA state.
            if (inputChar == 'a') newState.route_a.push_back(seenStates[resultSet]);
            else if (inputChar == 'b') newState.route_b.push_back(seenStates[resultSet]);
        }

        dfa.push_back(newState); // Add the newly created DFA state.
    }

    return dfa;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string filename = argv[1];
    
    vector<StateProps> nfa = readStatesFromFile(filename);
    cout << "Read " << nfa.size() << " states from the file." << endl;
    cout << "This is the NFA" << endl;
    // printStates(nfa);
    vector<StateProps> dfa=convertNFAtoDFA(nfa);
    cout << "This is the DFA" << endl;
    printStates(dfa);

    return 0;
}
