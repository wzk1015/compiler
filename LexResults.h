#pragma once

#include <iostream>
#include <utility>

#include "Error.h"

using namespace std;

class LexResults {
public:
    string type;
    string str;
    int v_int = -1;
    char v_char = 'E';
    int line{};
    int column{};
    int pos{};

    LexResults(string t, string s, int l, int c, int p) :
            type(std::move(t)), str(std::move(s)), line(l), column(c), pos(p) {};
    explicit LexResults(const string& t) : type(t){
        if (t != INVALID) {
            throw exception();
        }
    };
};

