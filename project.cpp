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
        string token;
        if (!(iss >> s.state >> token)) { 
            break; 
        }
        s.start = (token == "true");
        if (!(iss >> token)) {
            break;
        }
        s.finish = (token == "true");

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    string filename = argv[1];


    vector<StateProps> nfa = readStatesFromFile(filename);
    cout << "Read " << nfa.size() << " states from the file." << endl;
    printStates(nfa);

    return 0;
}
