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
    return closure;
}

string convertSetToStateName(const set<string>& stateSet) {
    string stateName;
    for (const auto& state : stateSet) {
        if (!stateName.empty()) {
            stateName += ",";
        }
        stateName += state;
    }
    cout << "[" + stateName + "]" << endl;
    return "[" + stateName + "]";
}


void addStateToDFA(vector<StateProps>& dfa, queue<set<string>>& stateQueue, set<string>& visitedStates, const set<string>& newState, bool isStart, bool isFinish) {
    string stateName = convertSetToStateName(newState);
    cout << "----" + stateName + "----";
    if (visitedStates.find(stateName) == visitedStates.end()) {
        StateProps dfaState;
        dfaState.state = stateName;
        dfaState.start = isStart;
        dfaState.finish = isFinish;
        dfa.push_back(dfaState);
        stateQueue.push(newState);
        visitedStates.insert(stateName);
    }
}

vector<StateProps> convertNFAtoDFA(const vector<StateProps>& nfa) {
    vector<StateProps> dfa;
    set<string> visitedStates;
    queue<set<string>> stateQueue;
    map<string, set<string>> transitionMapA, transitionMapB; 
    string initialState = findInitialState(nfa);
    set<string> initialSet = {initialState};
    stateQueue.push(initialSet);
    visitedStates.insert(initialState);
    while (!stateQueue.empty()) {
        set<string> currentStateSet = stateQueue.front();
        stateQueue.pop();
        set<string> newStateA, newStateB;
        bool isFinal = false;
        for (const string& state : currentStateSet) {
            for (const auto& nfaState : nfa) {
                if (state == nfaState.state) {
                    if (nfaState.finish) isFinal = true;
                    for (const auto& dest : nfaState.route_a) newStateA.insert(dest);
                    for (const auto& dest : nfaState.route_b) newStateB.insert(dest);
                }
            }
        }
        if (!newStateA.empty()) {
            string newStateNameA = convertSetToStateName(newStateA);
            transitionMapA[convertSetToStateName(currentStateSet)].insert(newStateNameA);
            addStateToDFA(dfa, stateQueue, visitedStates, newStateA, false, isFinalState(newStateNameA, nfa));
        }
        if (!newStateB.empty()) {
            string newStateNameB = convertSetToStateName(newStateB);
            transitionMapB[convertSetToStateName(currentStateSet)].insert(newStateNameB);
            addStateToDFA(dfa, stateQueue, visitedStates, newStateB, false, isFinalState(newStateNameB, nfa));
        }
    }
    for (auto& dfaState : dfa) {
        if (transitionMapA.find(dfaState.state) != transitionMapA.end()) {
            for (const auto& transState : transitionMapA[dfaState.state]) {
                dfaState.route_a.push_back(transState); // Convert set<string> to vector<string> if necessary
            }
        }
        if (transitionMapB.find(dfaState.state) != transitionMapB.end()) {
            for (const auto& transState : transitionMapB[dfaState.state]) {
                dfaState.route_b.push_back(transState);
            }
        }
    }

    return dfa;
}

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
