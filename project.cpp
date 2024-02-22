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
    // Initial state for DFA: combine all NFA start states
    string startState = "";
    for (const auto& state : nfa) {
        if (state.start) {
            if (!startState.empty()) startState += ",";
            startState += state.state;
        }
    }

    // Queue to manage states to process
    queue<string> toProcess;
    toProcess.push(startState);

    // Keep track of processed and to be processed states
    set<string> processed;
    processed.insert(startState);

    // Resulting DFA
    vector<StateProps> dfa;

    // Process states
    while (!toProcess.empty()) {
        string currentState = toProcess.front();
        toProcess.pop();

        StateProps newState;
        newState.state = currentState;
        newState.start = (currentState == startState); // Start state check
        newState.finish = false; // To be determined based on NFA finish states

        // Determine finish state and transitions for 'a' and 'b'
        set<string> aStates, bStates;
        vector<string> currentStates = split(currentState, ',');
        for (const auto& cs : currentStates) {
            // Find corresponding NFA state
            for (const auto& nfaState : nfa) {
                if (nfaState.state == cs) {
                    newState.finish |= nfaState.finish; // Finish state determination

                    // Transitions for 'a'
                    for (const auto& aState : nfaState.route_a) {
                        aStates.insert(aState);
                    }

                    // Transitions for 'b'
                    for (const auto& bState : nfaState.route_b) {
                        bStates.insert(bState);
                    }
                    break; // Break since state is found
                }
            }
        }

        // Convert sets to string and vector for newState
        newState.route_a = vector<string>(aStates.begin(), aStates.end());
        newState.route_b = vector<string>(bStates.begin(), bStates.end());

        // Add transitions states to process queue if not processed
        string aStateStr = join(newState.route_a, ",");
        string bStateStr = join(newState.route_b, ",");
        if (!aStateStr.empty() && processed.find(aStateStr) == processed.end()) {
            toProcess.push(aStateStr);
            processed.insert(aStateStr);
        }
        if (!bStateStr.empty() && processed.find(bStateStr) == processed.end()) {
            toProcess.push(bStateStr);
            processed.insert(bStateStr);
        }

        // Add newState to DFA
        dfa.push_back(newState);
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
