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
