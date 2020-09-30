#include <utility>

//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

#define NOTFOUND "NOT FOUND"
#define INVALID "NOT VALID"

class Error{
public:
    string msg;
    int line{};
    int column{};
    int eid;

    explicit Error(string s, int id): msg(move(s)), eid(id) {};
    Error(string s, int ln, int col, int id) : msg(std::move(s)), line(ln), column(col), eid(id) {};
};

class ErrorException : public exception {
    vector<Error> errors;
//    const char *what() const noexcept override {
//        //return ("Error in line " + to_string(ln) + ", column " + to_string(col) + ": " + move(s)).c_str();
//    }
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

#define DEBUG true

#endif