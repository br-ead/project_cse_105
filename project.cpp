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
    vector<StateProps> dfa;
    set<string> processedStates;

    // Step 1: Determine initial state of DFA (epsilon closure of initial states of NFA)
    set<string> initialClosure;
    for (const auto& state : nfa) {
        if (state.start) {
            initialClosure.insert(state.state);
        }
    }
    initialClosure = epsilonClosure(initialClosure, nfa);

    // Create initial state of DFA
    StateProps initialState;
    initialState.state = joinStates(initialClosure);
    initialState.start = true;
    initialState.finish = containsAcceptingState(initialClosure, nfa);
    dfa.push_back(initialState);
    processedStates.insert(initialState.state);

    // Step 2: State Expansion
    queue<string> stateQueue;
    stateQueue.push(initialState.state);
    while (!stateQueue.empty()) {
        string currentState = stateQueue.front();
        stateQueue.pop();

        // Step 3: Determine transitions on each input symbol
        map<char, set<string>> transitions;
        for (const auto& symbol : {'a', 'b'}) { // Assuming input alphabet is 'a' and 'b'
            set<string> nextStateSet;
            for (const auto& state : splitStates(currentState)) {
                nextStateSet = union(nextStateSet, moveOnInput(state, symbol, nfa));
            }
            nextStateSet = epsilonClosure(nextStateSet, nfa);
            transitions[symbol] = nextStateSet;

            // Step 4: Identify new DFA states
            string nextState = joinStates(nextStateSet);
            if (!nextState.empty() && processedStates.find(nextState) == processedStates.end()) {
                StateProps newState;
                newState.state = nextState;
                newState.finish = containsAcceptingState(nextStateSet, nfa);
                dfa.push_back(newState);
                processedStates.insert(nextState);
                stateQueue.push(nextState);
            }
        }

        // Assign transitions to current DFA state
        for (auto& state : dfa) {
            if (state.state == currentState) {
                state.route_a = joinStates(transitions['a']);
                state.route_b = joinStates(transitions['b']);
                break;
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
