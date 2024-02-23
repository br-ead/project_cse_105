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

/*
bool isFinalState(const string& state, const vector<StateProps>& dfa) {
    for (const auto& dfaState : dfa) {
        if (dfaState.state == state && dfaState.finish) {
            return true;
        }
    }
    return false;
}
Used Chat GPT here, but this is my isFinalState check */

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

vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;
    map<string, StateProps> stateMap; // Map to quickly find states by their name
    for (const auto& state : nfa) {
        stateMap[state.state] = state;
    }

    set<string> processed; // Keep track of processed states to avoid duplication
    queue<string> toProcess; // Queue for states to process

    string startState = findInitialState(nfa);
    toProcess.push(startState);
    processed.insert(startState);

    while (!toProcess.empty()) {
        string currentState = toProcess.front();
        toProcess.pop();

        // Create a new DFA state for the current set of NFA states
        StateProps newState;
        newState.state = currentState;
        newState.start = (currentState == startState);
        newState.finish = isFinalState(currentState, dfa); // Might need adjustment for composite states

        // For each input symbol, determine the new state
        set<string> newStatesA, newStatesB;
        stringstream ss(currentState);
        string token;
        while (getline(ss, token, '/')) {
            if (stateMap[token].route_a.size() > 0) {
                for (const auto& dest : stateMap[token].route_a) {
                    if (dest != "null") newStatesA.insert(dest);
                }
            }
            if (stateMap[token].route_b.size() > 0) {
                for (const auto& dest : stateMap[token].route_b) {
                    if (dest != "null") newStatesB.insert(dest);
                }
            }
        }

        // Convert sets to string representations for new states
        string newStateA = convertSetToStateName(newStatesA);
        string newStateB = convertSetToStateName(newStatesB);

        // Check if new states need processing
        if (newStatesA.size() > 0 && processed.find(newStateA) == processed.end()) {
            toProcess.push(newStateA);
            processed.insert(newStateA);
        }
        if (newStatesB.size() > 0 && processed.find(newStateB) == processed.end()) {
            toProcess.push(newStateB);
            processed.insert(newStateB);
        }

        // Store transitions in newState
        if (newStatesA.size() > 0) newState.route_a.push_back(newStateA);
        if (newStatesB.size() > 0) newState.route_b.push_back(newStateB);

        dfa.push_back(newState);
    }

    return dfa;
}
// implemented by chatGPt


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
    //cout << "This is the DFA" << endl;
    //printStates(dfa);

    return 0;
}
