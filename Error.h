
//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "utils.h"

#define NOTFOUND "NOT FOUND"
#define INVALID "NOT VALID"

#define DEBUG true

using namespace std;

class Error {
public:
    string msg;
    int line{};
    int column{};
    int eid;
    char err_code{};
    string rich_msg;

    explicit Error(string s, int id) : msg(move(s)), eid(id) {
        rich_msg = "Error: " + msg + " (EID: " + to_string(eid) + ")";
    };

    Error(string s, int ln, int col, int id) : msg(std::move(s)), line(ln), column(col), eid(id) {
        rich_msg = "Error in line " + to_string(ln) + ", column " + to_string(col)
                   + ": " + msg + " (EID: " + to_string(eid) + ")";
    }

    explicit Error(string s, char ch) : msg(move(s)), eid(1000 + ch), err_code(ch) {
        rich_msg = "Error: " + msg + " (EID: " + ch + ")";
    };

    Error(string s, int ln, int col, char ch) : msg(std::move(s)), line(ln), column(col),
                                                eid(1000 + ch), err_code(ch) {
        rich_msg = "Error in line " + to_string(ln) + ", column " + to_string(col)
                   + ": " + msg + " (EID: " + ch + ")";
    }
};

class Errors {
public:
    static vector<Error> errors;

    static void debug_add() {
        if (DEBUG) {
            cout << errors[errors.size() - 1].rich_msg << endl;
        }
    }

    static void add(const string &s, int line, int col, int id) {
        errors.emplace_back(s, line, col, id);
        debug_add();
    }

    static void add(const string &s, int id) {
        errors.emplace_back(s, id);
        debug_add();
    }

    static void add(const string &s, int line, int col, char ch) {
        errors.emplace_back(s, line, col, ch);
        debug_add();
    }

    static void add(const string &s, char ch) {
        errors.emplace_back(s, ch);
        debug_add();
    }

    static bool terminate() {
        if (errors.empty()) {
            cout << endl << "All correct." << endl;
            return false;
        }

        if (errors.size() == 1) {
            cerr << endl << "1 error. Listed as below." << endl;
        } else {
            cerr << errors.size() << " error(s). Listed as below." << endl;
        }

        for (auto &err: errors) {
            cerr << err.rich_msg << endl;
        }
        return true;
    }

    static void save_to_file(const string &out_path) {
        ofstream out(out_path);
        for (auto &err: errors) {
            if (err.eid > 1000 && err.line != 0) {
                out << err.line << " " << err.err_code << endl;
                if (DEBUG) {
                    cout << err.line << " " << err.err_code << endl;
                }
            }
        }
        out.close();
    }
};

#define E_EMPTY_FILE 1

//Lexer
#define E_UNEXPECTED_CHAR 2
#define E_UNEXPECTED_EOF 3

//Grammar
#define E_GRAMMAR 4


//error process homework
#define ERR_LEXER 'a'
#define ERR_REDEFINED 'b'
#define ERR_UNDEFINED 'c'
#define ERR_PARA_COUNT 'd'
#define ERR_PARA_TYPE 'e'
#define ERR_CONDITION_TYPE 'f'
#define ERR_NONRET_FUNC 'g'
#define ERR_RET_FUNC 'h'
#define ERR_INDEX_CHAR 'i'
#define ERR_CONST_ASSIGN 'j'
#define ERR_SEMICOL 'k'
#define ERR_RPARENT 'l'
#define ERR_RBRACK 'm'
#define ERR_ARRAY_INIT 'n'
#define ERR_CONST_TYPE 'o'
#define ERR_SWITCH_DEFAULT 'p'

#endif