#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <bitset>
#include <queue>
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
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string join(const vector<string>& elements, const string& delimiter) {
    if (elements.empty()) {
        return "";
    }

    std::string result = elements[0];
    for (size_t i = 1; i < elements.size(); ++i) {
        result += delimiter + elements[i];
    }

    return result;
}
vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    map<set<string>, StateProps> dfaStates; // Maps set of NFA states to DFA states
    queue<set<string>> processQueue; // Queue to process sets of NFA states
    set<string> startStateSet; // Set to hold the start state(s)

    // Find the start state and initialize the processQueue with it
    for (const auto& state : nfa) {
        if (state.start) {
            startStateSet.insert(state.state);
            break; // Assuming single start state for simplicity
        }
    }
    processQueue.push(startStateSet);
    dfaStates[startStateSet] = StateProps{join(startStateSet, ','), true, false, {}, {}};

    // Process each set of NFA states to create DFA states
    while (!processQueue.empty()) {
        auto currentSet = processQueue.front();
        processQueue.pop();
        map<char, set<string>> transitions; // Maps input symbol to resulting set of NFA states

        // Compute transitions for each state in the current set
        for (const auto& stateName : currentSet) {
            for (const auto& nfaState : nfa) {
                if (nfaState.state == stateName) {
                    // Check for finish state
                    if (nfaState.finish) {
                        dfaStates[currentSet].finish = true;
                    }

                    // Transition for 'a'
                    for (const auto& dest : nfaState.route_a) {
                        transitions['a'].insert(dest);
                    }

                    // Transition for 'b'
                    for (const auto& dest : nfaState.route_b) {
                        transitions['b'].insert(dest);
                    }
                    break; // Found the matching state, no need to continue the search
                }
            }
        }

        // Update DFA transitions and states based on computed transitions
        for (const auto& [input, destSet] : transitions) {
            if (!destSet.empty()) {
                // Generate a string representation for the new state
                string destStateName = join(destSet, ",");
                if (dfaStates.find(destSet) == dfaStates.end()) { // If this DFA state doesn't exist, create it
                    dfaStates[destSet] = StateProps{destStateName, false, false, {}, {}};
                    processQueue.push(destSet);
                }

                // Update the transition of the current DFA state
                if (input == 'a') {
                    dfaStates[currentSet].route_a.push_back(destStateName);
                } else if (input == 'b') {
                    dfaStates[currentSet].route_b.push_back(destStateName);
                }
            }
        }
    }

    // Convert map to vector
    vector<StateProps> dfa;
    for (const auto& [states, stateProps] : dfaStates) {
        dfa.push_back(stateProps);
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
