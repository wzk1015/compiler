//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

#define NOTFOUND "NOT FOUND"

class Error : public exception
{
public:
    string msg;
//    explicit Error(string s): msg(move(s)) {};
    Error(string s, int ln, int col):
    msg("Error in line " + to_string(ln) + ", column " + to_string(col) + ": " + move(s)) {};
    const char * what () const noexcept override { return msg.c_str(); }
};

#endif