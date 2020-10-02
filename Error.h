
//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

#define NOTFOUND "NOT FOUND"
#define INVALID "NOT VALID"

#define DEBUG true

#include <iostream>
#include <vector>

using namespace std;

class Error {
public:
    string msg;
    int line{};
    int column{};
    int eid;
    string rich_msg;

    explicit Error(string s, int id) : msg(move(s)), eid(id) {
        rich_msg = "Error: " + msg + " (EID: " + to_string(eid) + ")";
    };

    Error(string s, int ln, int col, int id) : msg(std::move(s)), line(ln), column(col), eid(id) {
        rich_msg = "Error in line " + to_string(ln) + ", column " + to_string(col)
                   + ": " + msg + " (EID: " + to_string(eid) + ")";
    }
};

class Errors {
public:
    static vector<Error> errors;
    static void add(const string& s, int ln, int col, int id) {
        errors.emplace_back(s, ln, col, id);
        if (DEBUG) {
            cout << errors[errors.size()-1].rich_msg << endl;
        }
    }
    static void add(const string& s, int id) {
        errors.emplace_back(s, id);
        if (DEBUG) {
            cout << errors[errors.size()-1].rich_msg << endl;
        }
    }
    static bool terminate() {
        if (errors.empty()) {
            cout << endl << "All correct." << endl;
            return false;
        }

        if (errors.size() == 1) {
            cout << endl << "1 error. Listed as below." << endl;
        } else {
            cout << errors.size() << " error(s). Listed as below." << endl;
        }

        for (auto &err: errors) {
            cout << err.rich_msg << endl;
        }
        return true;
    }
};

#define E_EMPTY_FILE 1

//Lexer
#define E_UNEXPECTED_CHAR 2
#define E_UNKNOWN_CHAR 3
#define E_UNEXPECTED_EOF 4

//Grammar
#define E_GRAMMAR 5

//Semantic
#define E_UNDEFINED_IDENTF 6



#endif