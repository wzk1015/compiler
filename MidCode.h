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
#define OP_END_FUNC "END_FUNC"


#define OP_PARA "PARA"
#define OP_PUSH "PUSH"
#define OP_CALL "CALL"
#define OP_RET "RET"
#define OP_VAR "VAR"
#define OP_CONST "CONST"
#define OP_EQL "EQL"
#define OP_NEQ "NEQ"
#define OP_GEQ "GEQ"
#define OP_GRE "GRE"
#define OP_LEQ "LEQ"
#define OP_LSS "LSS"
#define OP_GOTO "GOTO"
#define OP_BNZ "BNZ"
#define OP_BZ "BZ"
#define OP_ARR_LOAD "ARR_LOAD"
#define OP_ARR_SAVE "ARR_SAVE"

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

    string to_standard_format() const;
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

    static void interpret();

    static void show() {
        cout << "========MID CODES========" << endl;
        for (auto &c: codes) {
            cout << c.to_str() << endl;

        }
        cout << "=========================" << endl;
    }

    static void save_to_file(const string &out_path) {
        ofstream out(out_path);
        for (int i = 0; i < strcons.size(); i++) {
            out << "str" << i << ": " << strcons[i] << endl;
        }
        out << "===============" << endl;
        for (auto &c: codes) {
            out << c.to_str() << endl;
        }
        out.close();
    }

    static void show_standard_format() {
        cout << "========MID CODES========" << endl;
        for (auto &c: codes) {
            string format = c.to_standard_format();
            if (format != INVALID) {
                cout << format << endl;
            }
        }
        cout << "=========================" << endl;
    }

    static void save_to_file_standard_format(const string &out_path) {
        ofstream out(out_path);
        for (auto &c: codes) {
            string format = c.to_standard_format();
            if (format != INVALID) {
                cout << format << endl;
            }
        }
        out.close();
    }
};


#endif //COMPILER_MIDCODE_H
