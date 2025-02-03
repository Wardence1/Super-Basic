#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

string keywords[] = {"print", "goto"};

// Returns an entire string if quotes are used
// Strings return with a quotation mark at [0]
string getElement(istringstream &iss, string start) {
    if (start.find("\"") != string::npos) {
        string word, str = start;
        do {
            iss >> word;
            str += " " + word;
        } while (word.find("\"") == string::npos);
        return str.substr(0, str.size()-1);
    } else {
        return start;
    }
}


int main(int argc, char** argv) {

    /* Error Handling */
    if (argc <= 1) {
        fprintf(stderr, "Enter a file name.\n");
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        fprintf(stderr, "Couldn't read file: \"%s\"\n", argv[1]);
        return 1;
    }

    /* Tokenize Script */
    // @todo escape sequences
    vector<string> command, arg1, arg2;
    string line;
    while (getline(file, line)) {

        if (line == "") continue;

        istringstream iss(line);
        vector<string> tokens;
        string word;

        while (iss >> word) {

            tokens.push_back(word); // command

            if (iss >> word) {
                word = getElement(iss, word);
                tokens.push_back(word); // arg1
            }
            else
                tokens.push_back("");

            if (iss >> word) {
                word = getElement(iss, word);
                tokens.push_back(word); // arg2
            }
            else
                tokens.push_back("");
        }

        for (int i = 0; i < tokens.size(); i++) {
            command.push_back(tokens[i++]);
            arg1.push_back(tokens[i++]);
            arg2.push_back(tokens[i++]);
        }
    }

    /* Go Through Script */
    // @todo: error handling
    unsigned token = 0; // Current line of the script
    vector<string> strStk;
    vector<int> numStk;

    while (token < command.size()) {

        if (command[token] == "print") {
            cout << arg1[token].substr(1) << "\n";
        } else if (command[token] == "prints") {
            cout << strStk.front() << "\n";
        } else if (command[token] == "printn") {
            cout << numStk.front() << "\n";

        } else if (command[token] == "push") {
            arg1[token][0] == '\"' ? strStk.push_back(arg1[token].substr(1)) : numStk.push_back(stoi(arg1[token]));
        } else if (command[token] == "pops") {
            strStk.pop_back();
        } else if (command[token] == "popn") {
            numStk.pop_back();

        } else if (command[token] == "add") {

        } else if (command[token] == "inputs") {
            string input;
            getline(cin, input);
            strStk.push_back(input);
        } else if (command[token] == "inputn") {
            string input;
            getline(cin, input);
            numStk.push_back(stoi(input));

        } else {
            fprintf(stderr, "Invalid command: \"%s\"\n", command[token].c_str());
            return 1;
        }

        token++;
    }

    return 0;
}