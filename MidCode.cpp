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

string MidCode::to_standard_format() const {
    if (op == OP_FUNC) {
        return num1 + " " + num2 + "()";
    }
    if (op == OP_PARA) {
        return "para " + num1 + " " + num2;
    }
    return INVALID;
}

void MidCodeList::refactor() {
    for (auto &code : codes) {
        string n1 = code.num1;
        string n2 = code.num2;
        string op = code.op;
        string r = code.result;
        if (r == "#T4") {

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

void MidCodeList::interpret() {
    vector<MidCode> new_codes;
    int i = 0;
    while (codes[i].op != OP_FUNC) {
        if (codes[i].op != OP_ASSIGN) {
            panic("global code not assign statement: " + codes[i].to_str());
        }
        if (!begins_num(codes[i].num2)) {
            panic("not begins num: " + codes[i].num2);
        }
        SymTableItem *it1 = &SymTable::ref_search(GLOBAL, codes[i].num1);
        it1->const_value = codes[i].num2;
        new_codes.push_back(codes[i]);
        i++;
    }
    string cur_func = GLOBAL;
    for (; i < codes.size(); i++) {
        MidCode c = codes[i];
        SymTableItem *it1, *it2, *itr;
        if (!begins_num(c.num1)) {
            it1 = &SymTable::ref_search(cur_func, c.num1);
        }
        if (!begins_num(c.num2)) {
            it2 = &SymTable::ref_search(cur_func, c.num2);
        }
        if (!begins_num(c.result) && c.result != VACANT) {
            itr = &SymTable::ref_search(cur_func, c.result);
        }


        if (c.op == OP_SCANF) {
            //将所有值保存
            for (auto &item: SymTable::global) {
                if (item.modified) {
                    new_codes.emplace_back(OP_ASSIGN, item.name, item.const_value, VACANT);
                    item.modified = false;
                }
            }
            for (auto &item: SymTable::local[cur_func]) {
                if (item.modified && item.stiType != tmp) {
                    new_codes.emplace_back(OP_ASSIGN, item.name, item.const_value, VACANT);
                    item.modified = false;
                }
            }
            while (codes[i].op != OP_END_FUNC) {
                new_codes.push_back(codes[i]);
                i++;
            }
            new_codes.push_back(codes[i]);
        } else if (c.op == OP_PRINT) {
            if (c.num2 == "int" || c.num2 == "char") {
                if (it1->valid && it1->dataType == character) {
                    char ch = (char) stoi(it1->const_value);
                    MidCodeList::strcons.push_back("@" + string(&ch));
                } else if (c.num2 == "char") {
                    char ch = (char) stoi(c.num1);
                    MidCodeList::strcons.push_back("@" + string(&ch));
                } else if (it1->valid && it1->dataType == integer) {
                    MidCodeList::strcons.push_back(it1->const_value);
                } else {
                    MidCodeList::strcons.push_back(c.num1);
                }
                new_codes.emplace_back(OP_PRINT, to_string(MidCodeList::strcons.size()-1), "strcon", VACANT);
            } else {
                new_codes.push_back(codes[i]);
            }
        } else if (c.op == OP_END_FUNC) {
            for (auto &item: SymTable::global) {
                if (item.modified) {
                    new_codes.emplace_back(OP_ASSIGN, item.name, item.const_value, VACANT);
                    item.modified = false;
                }
            }
            new_codes.push_back(codes[i]);
        } else if (c.op == OP_FUNC) {
            cur_func = c.num2;
            new_codes.push_back(codes[i]);
        } else if (c.op == OP_ASSIGN) {
            it1->const_value = begins_num(c.num2) ? c.num2 : it2->const_value;
            it1->modified = true;
        } else if (c.op == OP_ADD || c.op == OP_SUB || c.op == OP_MUL || c.op == OP_DIV) {
            int v1 = stoi(begins_num(c.num1) ? c.num1 : it1->const_value);
            int v2 = stoi(begins_num(c.num2) ? c.num2 : it2->const_value);
            int r = c.op == OP_ADD ? v1 + v2 : c.op == OP_SUB ? v1 - v2 :
                                               c.op == OP_MUL ? v1 * v2 : c.op == OP_DIV ? v1 / v2 : 23333;
            itr->const_value = to_string(r);
            itr->modified = true;
        }
    }
    codes = new_codes;
}
