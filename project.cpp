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
#include <string>
#include <vector>

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

    // Process each state in the NFA
    for (const auto& qA : nfa) {
        for (const auto& qB : nfa) {
            // Combine states qA and qB into a new state
            StateProps newState;
            newState.state = "[" + qA.state + "," + qB.state + "]";
            newState.start = qA.start || qB.start;

            // Determine if either qA or qB is a final state
            newState.finish = qA.finish || qB.finish;

            // Determine the transitions for inputs 'a' and 'b' based on the NFA transitions
            for (const auto& routeA : qA.route_a) {
                for (const auto& routeB : qB.route_a) {
                    if (routeA == routeB) {
                        newState.route_a.push_back(routeA); // 'a' transition remains the same
                    }
                }
            }
            for (const auto& routeA : qA.route_b) {
                for (const auto& routeB : qB.route_b) {
                    if (routeA == routeB) {
                        newState.route_b.push_back(routeA); // 'b' transition remains the same
                    }
                }
            }

            // Sort the routes to eliminate duplicates
            sort(newState.route_a.begin(), newState.route_a.end());
            sort(newState.route_b.begin(), newState.route_b.end());

            // Add the new state to the DFA if not processed already
            string canonicalState = newState.state;
            sort(canonicalState.begin(), canonicalState.end()); // Sort the state representation
            if (processedStates.find(canonicalState) == processedStates.end()) {
                dfa.push_back(newState);
                processedStates.insert(canonicalState);
            }
        }
    }

    // Simplify state representations like q1,q1 to just q1
    for (auto& state : dfa) {
        auto pos = state.state.find(",");
        if (pos != string::npos && state.state.substr(1, pos - 1) == state.state.substr(pos + 1, state.state.size() - pos - 2)) {
            state.state = "[" + state.state.substr(1, pos - 1) + "]";
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
