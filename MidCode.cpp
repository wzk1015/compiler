//
// Created by wzk on 2020/11/5.
//

#include "MidCode.h"

vector<MidCode> MidCodeList::codes;
int MidCodeList::code_index = 1;
vector<string> MidCodeList::strcons;
int MidCodeList::strcon_index = 1;


string MidCode::to_str() const {
    if (result != VACANT) {
        return result + " = " + num1 + " " + op + " " + num2;
    }
    if (op == OP_ASSIGN) {
        return num1 + " = " + num2;
    }
    if (num2 == VACANT) {
        return op + " " + num1;
    }
    return op + " " + num1 + " " + num2;
}

void MidCodeList::refactor() {
    for (auto & code : codes) {
        string n1 = code.num1;
        string n2 = code.num2;
        string op = code.op;
        string r = code.result;
        if (r == "#T4"){

        }
        if (n1[0] == '\'') {
            code.num1 = to_string(n1[1]); //char
            n1 = code.num1;
        }
        if (n2[0] == '\'') {
            code.num2 = to_string(n2[1]); //char
            n2 = code.num2;
        }
        if (op == OP_ADD || op == OP_MUL) {
            if (begins_num(n1) && begins_num(n2)) {
                int result = (op == OP_ADD) ? stoi(n1) + stoi(n2) : stoi(n1) * stoi(n2);
                code = MidCode(OP_ASSIGN, r, to_string(result), VACANT);
            } else if (begins_num(n1)) {    //因为只允许addu $t1, $t2, 5，所以交换两个数顺序
                code.num1 = n2;
                code.num2 = n1;
                n1 = code.num1;
                n2 = code.num2;
            }
        }
        if (op == OP_SUB) {
            if (begins_num(n1) && begins_num(n2)) {
                code = MidCode(OP_ASSIGN, r, to_string(stoi(n1) - stoi(n2)), VACANT);
            } else if (begins_num(n1)) {    //y=5-x:  z=x-5; z=-y
                //Grammar里已处理
            }
        }
        if (op == OP_DIV) {
            if (begins_num(n1) && begins_num(n2)) {
                code = MidCode(OP_ASSIGN, r, to_string(stoi(n1) / stoi(n2)), VACANT);
            } else if (begins_num(n1)) {    //y=5/x:  z=5; y=z/x
                //Grammar里已处理
            }
        }
    }
}