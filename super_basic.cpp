#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

vector<string> strStk;
vector<int> numStk;
vector<pair<string, int>> labels;
vector<string> command, arg1, arg2;
bool carryFlag = false;

// Returns an entire string if quotes are used
string getElement(istringstream &iss, string start, unsigned lineNum) {
    // NOTE: Strings return with a quotation mark at [0]

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
                case 'r':
                    temp.push_back('\r');
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
        if ((str[i] < '-' || str[i] > '9') || str[i] == '`') {
            return false;
        }
    }
    return true;
}

bool setLower(string& str) {

    for (int i = 0; i < str.size(); i++) {
        str[i] = tolower(str[i]);
    }
    return true;
}

bool jump(unsigned& token, string curLabel) {
    setLower(curLabel);
    bool end = false;
    for (auto label : labels) {
        if (label.first == curLabel) {
            token = label.second+1;
            return true;
        }
    }
    fprintf(stderr, "Error: %s is not a registered label.\n", curLabel.c_str());
    exit(1);
}

int main(int argc, char** argv) {

    /* Error Handling */
    if (argc <= 1) {
        fprintf(stderr, "Enter a file name.\n");
        return 1;
    }

    /* Read the File */
    ifstream file(argv[1]);
    if (!file) {
        fprintf(stderr, "Couldn't read file: \"%s\"\n", argv[1]);
        return 1;
    }

    /* Tokenize Script */
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

    /* Command Declarations */
    for (int i = 0; i < command.size(); i++) {
        
        if (command[i][command[i].find_last_not_of(' ')] == ':') {
            setLower(command[i]);
            labels.push_back({command[i].substr(0, command[i].find_last_not_of(' ')), i});
        }
    }

    /* Go Through Script */
    // NOTE: token does not represent line count, commented/null lines are excluded during tokenization.
    unsigned token = 0; // Current command of the script

    jump(token, "start"); // Start at the start label

    while (token < command.size()) {

        /* Output */
        if (command[token] == "print") { // Print a string
            cout << arg1[token].substr(1);
        } else if (command[token] == "prints") { // Print the top of the string stack
            if (strStk.empty()) { fprintf(stderr, "Error, command \"prints\": Can't print from an empty stack.\n"); exit(-1); }
            cout << strStk.front();
            if (arg1[token] != "") cout << arg1[token].substr(1);
        } else if (command[token] == "printn") { // Print the top of the number stack
            if (numStk.empty()) { fprintf(stderr, "Error, command \"printn\": Can't print from an empty stack.\n"); exit(-1); };
            cout << numStk.front();
            if (arg1[token] != "")cout << arg1[token].substr(1);

        /* Input */
        } else if (command[token] == "inputs") { // Takes in a string as input from the user
            string input;
            getline(cin, input);
            strStk.push_back(input);
        } else if (command[token] == "inputn") { // Takes in a number as input from the user
            string input;
            getline(cin, input);
            if (input == "") input = "0";
            if (isNum(input)) {
                numStk.push_back(stoi(input));
            } else {
                fprintf(stderr, "Error: A number must be entered when \"inputn\" is called.\n");
                return 1;
            }

        /* Stack Operations */
        } else if (command[token] == "push") { // Pushes a string or number onto the stack, the stack is decided dynamically
            arg1[token][0] == '\"' ? strStk.push_back(arg1[token].substr(1)) : numStk.push_back(stoi(arg1[token]));
        } else if (command[token] == "pops") { // Pops the top of the string stack
            if (strStk.empty()) { fprintf(stderr, "Error, command \"pops\": Can't pop from an empty stack.\n"); exit(-1); };
            strStk.pop_back();
        } else if (command[token] == "popn") { // Pops the top of the number stack
            if (numStk.empty()) { fprintf(stderr, "Error, command \"popn\": Can't pop from an empty stack.\n"); exit(-1); };
            numStk.pop_back();

        /* Number Manipulation */
        } else if (command[token] == "add") { // Adds arg1 to the number at the top of the number stack
            if (isNum(arg1[token])) {
                numStk[numStk.size()-1] += stoi(arg1[token]);
            } else {
                fprintf(stderr, "Error, The first arg of \"add\" is a number.");
                return 1;
            }
        } else if (command[token] == "sub") { // Subtracts arg1 to the number at the top of the number stack
            if (isNum(arg1[token])) {
                numStk[numStk.size()-1] -= stoi(arg1[token]);
            } else {
                fprintf(stderr, "Error, The first arg of \"sub\" is a number.");
                return 1;
            }


        /* Comparing */
        } else if (command[token] == "cmp") { // arg1 is a symbol while arg2 is the number being compared to the top of the number stack with the symbol, the carry flag is set accordingly
            
            if (arg2[token] == "" || !isNum(arg2[token])) {
                fprintf(stderr, "Error: arg2 of \"cmp\" needs to contain a number.\n");
                exit(-1);
            }
            if (numStk.empty()) { fprintf(stderr, "Error, command \"cmp\": Can't compare to an empty stack.\n"); exit(-1); };

            int num = stoi(arg2[token]);

            if (arg1[token] == "=") {
                carryFlag = numStk.front() == num;
            } else if (arg1[token] == ">") {
                carryFlag = numStk.front() > num;
            } else if (arg1[token] == "<") {
                carryFlag = numStk.front() < num;
            }

        /* Control Flow */
        } else if (command[token] == "jmp") { // Jumps to the label specified at arg1
            
            jump(token, arg1[token]);
            continue;

        } else if (command[token] == "jmpt") { // Jumps to the label specified at arg1 if the carry flag is set to true
        
            if (carryFlag) {
                
                jump(token, arg1[token]);
                continue;
            }

        } else if (command[token] == "jmpf") { // Jumps to the label specified at arg1 if the carry flag is set to false
        
            if (!carryFlag) {
                
                jump(token, arg1[token]);
                continue;
            }

        /* System Commands */
        } else if (command[token] == "exit") { // Causes the program returns zero
            return 0;


        } else {

            // Check to see if the command is a label
            bool end = false;
            for (auto label : labels) {
                if (label.first + ":" == command[token]) {end = true; break;}
            }
            if (end) break;

            // Invalid command
            fprintf(stderr, "Invalid command: \"%s\"\n", command[token].c_str());
            exit(1);
        }

        token++;
    }

    return 0;
}