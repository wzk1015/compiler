//
// Created by wzk on 2020/10/24.
//

#include "utils.h"

string lower(string wd) {
    string s;
    int len = wd.size();
    for (int i = 0; i < len; i++) {
        if (wd[i] >= 'A' && wd[i] <= 'Z') {
            s += (char) (wd[i] + 'a' - 'A');
        } else {
            s += wd[i];
        }
    }
    return s;
}

bool is_2_power(int x){
    return (x&(x-1))==0;
}

bool begins_num(string symbol) {
    return isdigit(symbol[0]) || symbol[0] == '+' || symbol[0] == '-';
}

bool num_or_char(string symbol) {
    return begins_num(symbol) || symbol[0] == '\'';
}

void panic(const string& msg){
    cerr << msg << endl;
    throw exception();
}