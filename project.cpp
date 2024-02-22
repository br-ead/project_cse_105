#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <bitset>

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

    // Initialize the DFA with the start state
    StateProps startState;
    startState.state = nfa[0].state; // Assuming the first state of NFA is the start state
    startState.start = true;
    startState.finish = false;
    dfa.push_back(startState);

    // Process each state in the DFA
    for (size_t i = 0; i < dfa.size(); ++i) {
        StateProps& currentState = dfa[i];

        // Compute transitions for input 'a'
        set<string> nextStatesA;
        for (const auto& state : split(currentState.state, ',')) {
            auto it = find_if(nfa.begin(), nfa.end(), [&](const StateProps& s) { return s.state == state; });
            if (it != nfa.end()) {
                nextStatesA.insert(it->route_a.begin(), it->route_a.end());
            }
        }
        currentState.route_a = vector<string>(nextStatesA.begin(), nextStatesA.end());

        // Compute transitions for input 'b'
        set<string> nextStatesB;
        for (const auto& state : split(currentState.state, ',')) {
            auto it = find_if(nfa.begin(), nfa.end(), [&](const StateProps& s) { return s.state == state; });
            if (it != nfa.end()) {
                nextStatesB.insert(it->route_b.begin(), it->route_b.end());
            }
        }
        currentState.route_b = vector<string>(nextStatesB.begin(), nextStatesB.end());

        // Explore new states
        for (char input : {'a', 'b'}) {
            // Compute the next state
            vector<string> nextState;
            if (input == 'a') {
                nextState = currentState.route_a;
            } else {
                nextState = currentState.route_b;
            }

            // Construct the new state
            string newStateStr = join(nextState, ',');
            if (!newStateStr.empty()) {
                // Check if the new state is already in the DFA
                auto it = find_if(dfa.begin(), dfa.end(), [&](const StateProps& s) { return s.state == newStateStr; });
                if (it == dfa.end()) {
                    StateProps newState;
                    newState.state = newStateStr;
                    newState.start = false;
                    newState.finish = false;
                    dfa.push_back(newState);
                }
            }
        }
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
