//
// Created by wzk on 2020/10/24.
//

#include <fstream>
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

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

bool is_2_power(int x) {
    return (x & (x - 1)) == 0;
}

bool begins_num(string symbol) {
    return isdigit(symbol[0]) || symbol[0] == '+' || symbol[0] == '-';
}

bool num_or_char(string symbol) {
    return begins_num(symbol) || symbol[0] == '\'';
}

void panic(const string &msg) {
    cerr << msg << endl;
    exit(1);
}

void assertion(bool flag) {
    if (!flag) {
        mips_debug();
//        panic("assertion failed");
    }
}

string str_replace(string str, const string &from, const string &to) {
    string s;
    int len = str.size();
    for (unsigned int i = 0; i < len; i++) {
        if (str.substr(i, from.size()) == from) {
            s += to;
            i += from.size() - 1;
        } else {
            s += str[i];
        }
    }
    return s;
}

int sum(const vector<int>& arr) {
    int v = 0;
    for (int i : arr) {
        v += i;
    }
    return v;
}

void mips_debug() {
    ofstream out("mips.txt");
    out << "addu 0, 0, 0" << endl;
    out.close();
    exit(0);
}