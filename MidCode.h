//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_MIDCODE_H
#define COMPILER_MIDCODE_H

#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <map>

#include "utils.h"
#include "SymTable.h"

#define VACANT "#VACANT"
#define AUTO "#AUTO"
#define ENDL "#ENDL"

#define OP_PRINT "PRINT"
#define OP_SCANF "SCANF"
#define OP_ASSIGN ":="
#define OP_ADD "+"
#define OP_SUB "-"
#define OP_MUL "*"
#define OP_DIV "/"
#define OP_FUNC "FUNC"

using namespace std;

class MidCode {
public:
    string op;
    string num1;
    string num2;
    string result;

    MidCode(string op, string n1, string n2, string r) :
            op(std::move(op)), num1(std::move(n1)), num2(std::move(n2)), result(std::move(r)) {};

    MidCode() = default;

    string to_str() const;
};

class MidCodeList {
public:
    static vector<MidCode> codes;
    static int code_index;
    static vector<string> strcons;
    static int strcon_index;

    static string add(const string &op, const string &n1, const string &n2, const string &r) {
        string result = r;
        if (result == AUTO) {
            result = "#T" + to_string(code_index);
            code_index++;
        }
        codes.emplace_back(op, n1, n2, result);
        return result;
    }

    static void refactor();

    static void show() {
        cout << "========MID CODES========" << endl;
        for (auto &c: codes) {
            cout << c.to_str() << endl;
        }
        cout << "=========================" << endl;
    }

    static void save_to_file(const string &out_path) {
        ofstream out(out_path);
        for (auto &c: codes) {
            out << c.to_str() << endl;
        }
        out.close();
    }
};


#endif //COMPILER_MIDCODE_H
