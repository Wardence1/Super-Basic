#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

string keywords[] = {"print", "goto"};

// Returns an entire string if quotes are used
// Strings return with a quotation mark at [0]
string getElement(istringstream &iss, string start, unsigned lineNum) {
    if (start.find("\"") != string::npos) {
        string word, str = start;

        // Check to see if string ends in the same word
        bool done = false;
        if (start.substr(start.find("\"")+1).find("\"") != string::npos) {
            done = true;
            str = str.substr(0, str.size()-1);    
        }

        do {
            if (!(iss >> word) && !done) {
                fprintf(stderr, "Error, line %d: Missing quote.\n", lineNum);
                exit(-1);
            }

            str += " " + word;
            if (word.find("\"") != string::npos) done = true;
        } while (!done);
        return str.substr(0, str.size()-1);
    } else {
        return start;
    }
}

// Adds escape sequences to strings
string parseString(string str, unsigned lineNum) {

    if (str == "") return str;
    string temp = "\"";

    for (int i = 1; i < str.size(); i++) {

        if (str[i] == '\\') {
            if (++i == str.size()) {
                fprintf(stderr, "Error, line %d: Line ends with \"\\\".\n", lineNum); 
                exit(-1);
            }
            switch(str[i]) {
                case 'n':
                    temp.push_back('\n');
                    break;
                case 't':
                    temp.push_back('\t');
                    break;
                case '\\':
                    temp.push_back('\\');
                    break;
                default:
                    fprintf(stderr, "\'%c\' is not a valid escape sequence.\n", str[i]);
            }
            continue;
        }
        temp.push_back(str[i]);
    }
    return temp;
}

bool isNum(string str) {
    for (int i = 0; i < str.size(); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
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
    int lineNum = 0;
    while (getline(file, line)) {

        lineNum++;
        if (line == "") continue;
        if (line[line.find_first_not_of(' ')] == ';') continue;

        istringstream iss(line);
        vector<string> tokens;
        string word;

        while (iss >> word) {

            tokens.push_back(word); // command

            if (iss >> word) {
                word = getElement(iss, word, lineNum);
                tokens.push_back(word); // arg1
            }
            else {
                tokens.push_back("");
            }

            if (iss >> word) {
                word = getElement(iss, word, lineNum);
                tokens.push_back(word); // arg2
            }
            else {
                tokens.push_back("");
            }
        }

        for (int i = 0; i < tokens.size(); i++) {
            command.push_back(tokens[i++]);
            arg1.push_back(tokens[i++]);
            arg2.push_back(tokens[i++]);
        }
    }

    /* Parse Strings */
    for (int i = 0; i < command.size(); i++) {
        if (arg1[i][0] == '\"') {
            arg1[i] = parseString(arg1[i], i);
        }
        if (arg2[i][0] == '\"') {
            arg2[i] = parseString(arg2[i], i);
        }
    }

    /* Go Through Script */
    // Note: token does not represent line count, commented/null lines are exluded during tokenization.
    unsigned token = 0; // Current command of the script
    vector<string> strStk;
    vector<int> numStk;

    while (token < command.size()) {

        if (command[token] == "print") {
            cout << arg1[token].substr(1);
        } else if (command[token] == "prints") {
            if (strStk.empty()) { fprintf(stderr, "Error, command \"prints\": Can't print from an empty stack.\n"); exit(-1); }
            cout << strStk.front();
            if (arg1[token] != "") cout << arg1[token].substr(1);
        } else if (command[token] == "printn") {
            if (numStk.empty()) { fprintf(stderr, "Error, command \"printn\": Can't print from an empty stack.\n"); exit(-1); };
            cout << numStk.front();
            if (arg1[token] != "")cout << arg1[token].substr(1);

        } else if (command[token] == "push") {
            arg1[token][0] == '\"' ? strStk.push_back(arg1[token].substr(1)) : numStk.push_back(stoi(arg1[token]));
        } else if (command[token] == "pops") {
            if (strStk.empty()) { fprintf(stderr, "Error, command \"pops\": Can't pop from an empty stack.\n"); exit(-1); };
            strStk.pop_back();
        } else if (command[token] == "popn") {
            if (numStk.empty()) { fprintf(stderr, "Error, command \"popn\": Can't pop from an empty stack.\n"); exit(-1); };
            numStk.pop_back();

        } else if (command[token] == "add") {
            if (isNum(arg1[token])) {
                numStk[numStk.size()-1] += stoi(arg1[token]);
            } else {
                fprintf(stderr, "Error, The first arg of \"add\" is a number.");
                return 1;
            }
        } else if (command[token] == "sub") {
            if (isNum(arg1[token])) {
                numStk[numStk.size()-1] -= stoi(arg1[token]);
            } else {
                fprintf(stderr, "Error, The first arg of \"sub\" is a number.");
                return 1;
            }

        } else if (command[token] == "inputs") {
            string input;
            getline(cin, input);
            strStk.push_back(input);
        } else if (command[token] == "inputn") {
            string input;
            getline(cin, input);
            if (input == "") input = "0";
            if (isNum(input)) {
                numStk.push_back(stoi(input));
            } else {
                fprintf(stderr, "Error: A number must be entered when \"inputn\" is called.\n");
                return 1;
            }

        } else {
            fprintf(stderr, "Invalid command: \"%s\"\n", command[token].c_str());
            return 1;
        }

        token++;
    }

    return 0;
}