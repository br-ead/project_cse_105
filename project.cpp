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
