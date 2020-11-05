//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_MIDCODE_H
#define COMPILER_MIDCODE_H

#include <iostream>
#include <utility>
#include <vector>

#define VACANT "-"
#define AUTO "auto"

#define OP_PRINT "print"
#define OP_SCANF "scanf"
#define OP_ASSIGN ":="
#define OP_ADD "+"
#define OP_SUB "-"
#define OP_MUL "*"
#define OP_DIV "/"

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

    string to_str() const {
        return op + ", " + num1 + ", " + num2 + ", " + result;
    }
};

class MidCodeList {
public:
    static vector<MidCode> codes;
    static int code_index;

    static string add(const string& op, const string& n1, const string& n2, const string& r) {
        string result = r;
        if (result == AUTO) {
            result = "T" + to_string(code_index);
            code_index++;
        }
        codes.emplace_back(op, n1, n2, result);
        return result;
    }

    static void show() {
        cout << "MID CODES:" << endl;
        for (auto& c: codes) {
            cout << c.to_str() << endl;
        }
    }
};





#endif //COMPILER_MIDCODE_H
