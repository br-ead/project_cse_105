#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
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
vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;

    // Generate all possible combinations of states
    for (int i = 0; i < (1 << nfa.size()); i++) {
        StateProps newState;
        newState.state = "";
        newState.start = false;
        newState.finish = false;

        // For each bit set in the combination, add the corresponding state to the new state
        for (int j = 0; j < nfa.size(); j++) {
            if (i & (1 << j)) {
                newState.state += nfa[j].state + (newState.state.empty() ? "" : ",");
                newState.start |= nfa[j].start;
                newState.finish |= nfa[j].finish;
            }
        }

        // Add the new state to the DFA
        dfa.push_back(newState);
    }

    // Define the transition function for the DFA
    for (StateProps& dfaState : dfa) {
        set<string> nextStatesA, nextStatesB;

        // For each NFA state in the current DFA state
        for (const string& nfaState : split(dfaState.state, ',')) {
            // Find the corresponding NFA state
            auto it = find_if(nfa.begin(), nfa.end(), &nfaState { return s.state == nfaState; });
            if (it != nfa.end()) {
                // Add all reachable states to the set of next states for the current input
                nextStatesA.insert(it->route_a.begin(), it->route_a.end());
                nextStatesB.insert(it->route_b.begin(), it->route_b.end());
            }
        }

        // The next state for the current input is the DFA state representing the set of next states
        dfaState.route_a = vector<string>(nextStatesA.begin(), nextStatesA.end());
        dfaState.route_b = vector<string>(nextStatesB.begin(), nextStatesB.end());
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
    cout << "This is the NFA" << endl;
    printStates(nfa);
    vector<StateProps> dfa=convertNFAtoDFA(nfa);
    cout << "This is the DFA" << endl;
    printStates(dfa);

    return 0;
}
