#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

string keywords[] = {"print", "goto"};

// Returns an entire string if quotes are used
string getElement(istringstream &iss, string start) {
    if (start.find("\"") != string::npos) {
        string word, str = start.substr(1);
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
    unsigned token = 0; // Current line of the script
    string currentStr(""); // Current string in focus from the command "read"

    while (token < command.size()) {

        if (command[token] == "print") {
            arg1[token] == "" ? cout << currentStr << "\n" : cout << arg1[token] << "\n";
        } else if (command[token] == "read") {
            getline(cin, currentStr);
        } else {
            fprintf(stderr, "Invalid command: \"%s\"\n", command[token].c_str());
            return 1;
        }

        token++;
    }

    return 0;
}