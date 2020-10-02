#pragma once

#include <iostream>
#include <utility>

#include "Error.h"

using namespace std;

class Token {
public:
    string type;
    string str;
    int v_int = -1;
    char v_char = 'E';
    int line{};
    int column{};
    int pos{};

    Token(string t, string s, int l, int c, int p) :
            type(std::move(t)), str(std::move(s)), line(l), column(c), pos(p) {};
    explicit Token(const string& t) : type(t){
        if (t != INVALID) {
            throw exception();
        }
    };
};

