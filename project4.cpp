#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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
        string start_str, finish_str, token;
        if (!(iss >> s.state >> start_str >> finish_str)) { 
            break; 
        }
        s.start = (start_str == "true");
        s.finish = (finish_str == "true");

        // Read routes
        while (getline(iss, token, '-')) {
            if (token == "x") {
                while (getline(iss, token, '-') && token != "y") {
                    s.route_a.push_back(token);
                }
            } else if (token != "y") {
                s.route_b.push_back(token);
            }
        }

        states.push_back(s);
    }

    return states;
}

void printStates(const vector<StateProps>& states) {
    for (const auto& state : states) {
        cout << "State " << state.state << " is " << (state.start ? "" : "not ") << "a start state. ";
        cout << "It is " << (state.finish ? "" : "not ") << "a finish state. When the input is a, it will route to ";
        
        if (state.route_a.empty()) {
            cout << "nothing";
        } else {
            for (const auto& route : state.route_a) {
                cout << route << "/";
            }
        }

        cout << ". When the input is b, it will route to ";
        
        if (state.route_b.empty()) {
            cout << "nothing";
        } else {
            for (const auto& route : state.route_b) {
                cout << route << "/";
            }
        }

        cout << endl;
    }
}

vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;

    for (const auto& qA : nfa) {
        for (const auto& qB : nfa) {
            // Check if qA is routed to qB and itself with the same input
            if ((find(qA.route_a.begin(), qA.route_a.end(), qB.state) != qA.route_a.end() && 
                 find(qA.route_a.begin(), qA.route_a.end(), qA.state) != qA.route_a.end()) ||
                (find(qA.route_b.begin(), qA.route_b.end(), qB.state) != qA.route_b.end() && 
                 find(qA.route_b.begin(), qA.route_b.end(), qA.state) != qA.route_b.end())) {

                // Combine qA and qB into a new state
                StateProps newState;
                newState.state = qA.state + "," + qB.state;
                newState.start = qA.start || qB.start;
                newState.finish = qA.finish || qB.finish;

                // Merge the routes of qA and qB
                newState.route_a.insert(newState.route_a.end(), qA.route_a.begin(), qA.route_a.end());
                newState.route_a.insert(newState.route_a.end(), qB.route_a.begin(), qB.route_a.end());
                newState.route_b.insert(newState.route_b.end(), qA.route_b.begin(), qA.route_b.end());
                newState.route_b.insert(newState.route_b.end(), qB.route_b.begin(), qB.route_b.end());

                // Add the new state to the DFA
                dfa.push_back(newState);
            }
        }
    }

    return dfa;
}

/*
vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;
    set<vector<string>> processedRoutes;

    // Process each state in the NFA
    for (const auto& qA : nfa) {
        for (const auto& qB : nfa) {
            // Combine states qA and qB into a new state
            StateProps newState;
            newState.state = "l" + qA.state + "," + qB.state + "l";
            newState.start = qA.start || qB.start;
            newState.finish = qA.finish || qB.finish;

            // Merge routes for input 'a'
            for (const auto& routeA : qA.route_a) {
                for (const auto& routeB : qB.route_a) {
                    vector<string> mergedRoute = { routeA, routeB };
                    sort(mergedRoute.begin(), mergedRoute.end());
                    processedRoutes.insert(mergedRoute);
                }
            }

            // Merge routes for input 'b'
            for (const auto& routeA : qA.route_b) {
                for (const auto& routeB : qB.route_b) {
                    vector<string> mergedRoute = { routeA, routeB };
                    sort(mergedRoute.begin(), mergedRoute.end());
                    processedRoutes.insert(mergedRoute);
                }
            }

            // Update the new state's routes
            for (const auto& route : processedRoutes) {
                if (route.size() == 2) {
                    newState.route_a.push_back(route[0]);
                    newState.route_b.push_back(route[1]);
                }
            }

            // Add the new state to the DFA if not processed already
            if (!newState.route_a.empty() || !newState.route_b.empty()) {
                dfa.push_back(newState);
            }

            // Clear processed routes for next iteration
            processedRoutes.clear();
        }
    }

    return dfa;
}
*/
vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;
    set<set<string>> markedStates;  // Set of marked states in the DFA

    // Create a start state for the DFA
    set<string> startState = {nfa[0].state};  // Assuming the first state in 'nfa' is the start state
    markedStates.insert(startState);

    // Process each state in the DFA
    for (const auto& dfaState : markedStates) {
        StateProps newState;
        newState.state = joinStates(dfaState);  // Function to join a set of states into a string
        newState.start = containsStartState(dfaState, nfa);  // Function to check if 'dfaState' contains a start state of the NFA
        newState.finish = containsFinishState(dfaState, nfa);  // Function to check if 'dfaState' contains a finish state of the NFA

        // Process each possible input symbol
        for (char symbol : {'a', 'b'}) {  // Assuming 'a' and 'b' are the input symbols
            set<string> nextState;

            // Compute the next state for the current DFA state and input symbol
            for (const string& nfaState : dfaState) {
                const vector<string>& routes = (symbol == 'a') ? getState(nfaState, nfa).route_a : getState(nfaState, nfa).route_b;  // Function to get the state in 'nfa' with the name 'nfaState'
                nextState.insert(routes.begin(), routes.end());
            }

            // If this set of NFA states is not already a state in the DFA, add it
            if (markedStates.find(nextState) == markedStates.end()) {
                markedStates.insert(nextState);
            }

            // Add the transition to the DFA
            (symbol == 'a') ? newState.route_a.push_back(joinStates(nextState)) : newState.route_b.push_back(joinStates(nextState));
        }

        // Add the new state to the DFA
        dfa.push_back(newState);
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
    printStates(nfa);

    return 0;
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
