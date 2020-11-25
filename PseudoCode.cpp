//
// Created by wzk on 2020/11/5.
//

#include "PseudoCode.h"

vector<PseudoCode> PseudoCodeList::codes;
int PseudoCodeList::code_index = 1;
vector<string> PseudoCodeList::strcons;
int PseudoCodeList::label_index = 1;
vector<int> PseudoCodeList::basic_block_idx;


string PseudoCode::to_str() const {
    if (op == OP_FUNC || op == OP_END_FUNC) {
        return "=========" + op + " " + num1 + " " + num2 + "=========";
    }
    if (op == OP_ASSIGN) {
        return num1 + " = " + num2;
    }
    if (op == OP_ARR_LOAD) {
        return result + " = " + num1 + "[" + num2 + "]";
    }
    if (op == OP_ARR_SAVE) {
        return num1 + "[" + num2 + "] = " + result;
    }
    if (op == OP_JUMP_IF) {
        return op + " " + num1 + num2 + " " + result;
    }
    if (result != VACANT) {
        return result + " = " + num1 + " " + op + " " + num2;
    }
    if (num1 == VACANT && num2 == VACANT) {
        return op;
    }
    if (num2 == VACANT) {
        return op + " " + num1;
    }
    return op + " " + num1 + " " + num2;
}

string PseudoCode::to_standard_format() const {
    if (op == OP_FUNC) {
        return num1 + " " + num2 + "()";
    }
    return INVALID;
}

void PseudoCodeList::refactor() {
    string cur_func = GLOBAL;
    for (auto &code : codes) {
        string n1 = code.num1;
        string n2 = code.num2;
        string op = code.op;
        string r = code.result;
        if (op == OP_FUNC) {
            cur_func = code.num2;
        }
        if (n1[0] == '\'') {
            code.num1 = to_string(n1[1]); //char
            n1 = code.num1;
        }
        if (n2[0] == '\'') {
            code.num2 = to_string(n2[1]); //char
            n2 = code.num2;
        }
        if (r[0] == '\'') {
            code.result = to_string(r[1]); //char
            r = code.result;
        }
        if (op == OP_ADD || op == OP_MUL) {
            if (begins_num(n1) && begins_num(n2)) {
                int result = (op == OP_ADD) ? stoi(n1) + stoi(n2) : stoi(n1) * stoi(n2);
                code = PseudoCode(OP_ASSIGN, r, to_string(result), VACANT);
            } else if (begins_num(n1)) {    //因为只允许addu $t1, $t2, 5，所以交换两个数顺序
                code.num1 = n2;
                code.num2 = n1;
                n1 = code.num1;
                n2 = code.num2;
            }
        }
        if (op == OP_SUB) {
            if (begins_num(n1) && begins_num(n2)) {
                code = PseudoCode(OP_ASSIGN, r, to_string(stoi(n1) - stoi(n2)), VACANT);
            } else if (begins_num(n1)) {    //y=5-x:  z=x-5; z=-y
                //Grammar里已处理
            }
        }
        if (op == OP_DIV) {
            if (begins_num(n1) && begins_num(n2)) {
                code = PseudoCode(OP_ASSIGN, r, to_string(stoi(n1) / stoi(n2)), VACANT);
            } else if (begins_num(n1)) {    //y=5/x:  z=5; y=z/x
                //Grammar里已处理
            }
        }
    }
}

void PseudoCodeList::remove_redundant_assign() {
    vector<PseudoCode> new_codes;
    for (int i = 0; i < codes.size() - 1; i++) {
        PseudoCode c1 = codes[i];
        PseudoCode c2 = codes[i + 1];
        if (c2.op == OP_ASSIGN && c2.num1[0] != '#' && c1.result[0] == '#' && c1.result == c2.num2 &&
            (is_arith(c1.op) || c1.op == OP_ARR_LOAD)) {
            new_codes.emplace_back(c1.op, c1.num1, c1.num2, c2.num1);
            i++;
        } else if (c1.op == OP_ASSIGN && c1.num1[0] == '#' && c1.num1 == c2.num1 &&
                   (is_arith(c2.op) || c2.op == OP_ARR_SAVE)) {
            new_codes.emplace_back(c2.op, c1.num2, c2.num2, c2.result);
            i++;
        } else if (c1.op == OP_ASSIGN && c1.num1[0] == '#' && c1.num1 == c2.num2 &&
                   (is_arith(c2.op) || c2.op == OP_ARR_SAVE)) {
            new_codes.emplace_back(c2.op, c2.num1, c1.num2, c2.result);
            i++;
        } else if (c1.op == OP_PRINT && c2.op == OP_PRINT && c1.num2 == "strcon" && c2.num1 == ENDL) {
            strcons[stoi(c1.num1)] += "\\n";
            new_codes.emplace_back(OP_PRINT, c1.num1, c1.num2, VACANT);
            i++;
        } else if (i == codes.size() - 2) {
            new_codes.push_back(c1);
            new_codes.push_back(c2);
        } else {
            new_codes.push_back(c1);
        }
    }
    codes = new_codes;
}

void PseudoCodeList::const_broadcast() {
    string cur_func = GLOBAL;
    vector<PseudoCode> new_codes;
    for (int i = 0; i < codes.size() - 1; i++) {
        PseudoCode c = codes[i];

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
        bool num1_can_cal = begins_num(c.num1) || !it1->const_value.empty();
        bool num2_can_cal = begins_num(c.num2) || !it2->const_value.empty();

        if (c.op == OP_ASSIGN && num2_can_cal && it1->stiType == tmp) {
            it1->const_value = begins_num(c.num2) ? c.num2 : it2->const_value;
        } else if (num1_can_cal && num2_can_cal) {
            int v1 = stoi(begins_num(c.num1) ? c.num1 : it1->const_value);
            int v2 = stoi(begins_num(c.num2) ? c.num2 : it2->const_value);
            if (is_arith(c.op)) {
                int r = c.op == OP_ADD ? v1 + v2 : c.op == OP_SUB ? v1 - v2 :
                                                   c.op == OP_MUL ? v1 * v2 : c.op == OP_DIV ? v1 / v2 : 23333;
                if (itr->stiType == tmp) {
                    itr->const_value = to_string(r);
                } else {
                    new_codes.emplace_back(OP_ASSIGN, c.result, to_string(r), VACANT);
                }
            } else {
                new_codes.emplace_back(c.op, to_string(v1), to_string(v2), c.result);
            }

        } else if (num1_can_cal) {
            int v1 = stoi(begins_num(c.num1) ? c.num1 : it1->const_value);

            if (c.op == OP_SUB || c.op == OP_DIV) {
                if (c.op == OP_DIV && v1 == 0) {
                    //y=0/x: y=0
                    if (itr->stiType == tmp) {
                        itr->const_value = "0";
                    } else {
                        new_codes.emplace_back(OP_ASSIGN, c.result, "0", VACANT);
                    }
                } else {
                    // y=5-x  y=5/x
                    if (c.num1 != to_string(v1)) {
                        new_codes.emplace_back(OP_ASSIGN, c.num1, to_string(v1), VACANT);
                    }
                    new_codes.push_back(c);
                }
            } else if ((c.op == OP_ADD && v1 == 0) || (c.op == OP_MUL && v1 == 1)) {
                //y=0+x: y=x
                new_codes.emplace_back(OP_ASSIGN, c.result, c.num2, VACANT);
            } else if ((c.op == OP_MUL && v1 == 0)) {
                //y=0*x: y=0
                if (itr->stiType == tmp) {
                    itr->const_value = "0";
                } else {
                    new_codes.emplace_back(OP_ASSIGN, c.result, "0", VACANT);
                }
            } else {
                new_codes.emplace_back(c.op, to_string(v1), c.num2, c.result);
            }

        } else if (num2_can_cal) {
            int v2 = stoi(begins_num(c.num2) ? c.num2 : it2->const_value);
            if ((c.op == OP_ADD && v2 == 0) || (c.op == OP_SUB && v2 == 0) ||
                (c.op == OP_MUL && v2 == 1) || (c.op == OP_DIV && v2 == 1)) {
                //y=a+0, y=a-0, y=a*1, y=a/1
                new_codes.emplace_back(OP_ASSIGN, c.result, c.num1, VACANT);
            } else if ((c.op == OP_MUL && v2 == 0)) {
                //y=a*0
                if (itr->stiType == tmp) {
                    itr->const_value = "0";
                } else {
                    new_codes.emplace_back(OP_ASSIGN, c.result, "0", VACANT);
                }
            } else {
                new_codes.emplace_back(c.op, c.num1, to_string(v2), c.result);
            }
        } else if (c.op == OP_FUNC) {
            cur_func = c.num2;
            new_codes.push_back(c);
        } else {
            new_codes.push_back(c);
        }
    }
    codes = new_codes;
}

void PseudoCodeList::remove_redundant_tmp() {

    bool modified;
    do {
        string cur_func = GLOBAL;
        vector<PseudoCode> new_codes;
        modified = false;
        for (int i = 0; i < codes.size() - 1; i++) {
            PseudoCode c1 = codes[i];
            PseudoCode c2 = codes[i + 1];
            SymTableItem *it11, *it12, *it1r;
            SymTableItem *it21, *it22, *it2r;
            if (!begins_num(c1.num1)) {
                it11 = &SymTable::ref_search(cur_func, c1.num1);
            }
            if (!begins_num(c1.num2)) {
                it12 = &SymTable::ref_search(cur_func, c1.num2);
            }
            if (!begins_num(c1.result) && c1.result != VACANT) {
                it1r = &SymTable::ref_search(cur_func, c1.result);
            }
            if (!begins_num(c2.num1)) {
                it21 = &SymTable::ref_search(cur_func, c2.num1);
            }
            if (!begins_num(c2.num2)) {
                it22 = &SymTable::ref_search(cur_func, c2.num2);
            }
            if (!begins_num(c2.result) && c2.result != VACANT) {
                it2r = &SymTable::ref_search(cur_func, c2.result);
            }

            if (c1.op != OP_ASSIGN && c2.op != OP_ASSIGN && c2.result != VACANT &&
                !begins_num(c2.result) && !begins_num(c1.result) && !begins_num(c2.num1) &&
                it2r->stiType == tmp && it21->stiType == tmp && it1r->name == it21->name
                && begins_num(c1.num2) && begins_num(c2.num2)) {
                if (c1.op == c2.op) {
                    if (c1.op == OP_ADD || c1.op == OP_SUB) {
                        new_codes.emplace_back(c1.op, c1.num1, to_string(stoi(c1.num2) + stoi(c2.num2)), c2.result);
                        modified = true;
                        i++;
                    } else if (c1.op == OP_MUL || c1.op == OP_DIV) {
                        new_codes.emplace_back(c1.op, c1.num1, to_string(stoi(c1.num2) * stoi(c2.num2)), c2.result);
                        modified = true;
                        i++;
                    } else {
                        new_codes.push_back(c1);
                    }
                } else if ((c1.op == OP_ADD && c2.op == OP_SUB) || (c1.op == OP_SUB && c2.op == OP_ADD)) {
                    new_codes.emplace_back(c1.op, c1.num1, to_string(stoi(c1.num2) - stoi(c2.num2)), c2.result);
                    modified = true;
                    i++;
                }

//                else if (((c1.op == OP_MUL && c2.op == OP_DIV) || (c1.op == OP_DIV && c2.op == OP_MUL)) &&
//                           (c1.num2 == c2.num2) && stoi(c1.num2) <= 2) {
//                    new_codes.emplace_back(OP_ASSIGN, c2.result, c1.num1, VACANT);
//                    modified = true;
//                    i++;
//                }

                else {
                    new_codes.push_back(c1);
                }

            } else if (c1.op == OP_FUNC) {
                cur_func = c1.num2;
                new_codes.push_back(c1);
            } else if (i == codes.size() - 2) {
                new_codes.push_back(c1);
                new_codes.push_back(c2);
            } else {
                new_codes.push_back(c1);
            }
        }
        codes = new_codes;
    } while (modified);
}

void PseudoCodeList::remove_tripple() {
    string cur_func = GLOBAL;
    vector<PseudoCode> new_codes;
    for (int i = 0; i < codes.size() - 2; i++) {
        PseudoCode c1 = codes[i];
        PseudoCode c2 = codes[i + 1];
        PseudoCode c3 = codes[i + 2];
        SymTableItem *it11, *it12, *it1r;
        SymTableItem *it21, *it22, *it2r;
        if (!begins_num(c1.num1)) {
            it11 = &SymTable::ref_search(cur_func, c1.num1);
        }
        if (!begins_num(c1.num2)) {
            it12 = &SymTable::ref_search(cur_func, c1.num2);
        }
        if (!begins_num(c1.result) && c1.result != VACANT) {
            it1r = &SymTable::ref_search(cur_func, c1.result);
        }
        if (!begins_num(c2.num1)) {
            it21 = &SymTable::ref_search(cur_func, c2.num1);
        }
        if (!begins_num(c2.num2)) {
            it22 = &SymTable::ref_search(cur_func, c2.num2);
        }
        if (!begins_num(c2.result) && c2.result != VACANT) {
            it2r = &SymTable::ref_search(cur_func, c2.result);
        }
        if (is_arith(c1.op) && c1.op == c2.op && c1.num1 == c2.num1 && c1.num2 == c2.num2 &&
                it1r->stiType == tmp && it2r->stiType == tmp && c3.num1 == c1.result && c3.num2 == c2.result) {
            if (c3.op == OP_SUB) {
                new_codes.emplace_back(OP_ASSIGN, c3.result, "0", VACANT);
                i += 2;
            } else if (c3.op == OP_DIV) {
                new_codes.emplace_back(OP_ASSIGN, c3.result, "1", VACANT);
                i += 2;
            }
        } else if (i == codes.size() - 3) {
            new_codes.push_back(c1);
            new_codes.push_back(c2);
            new_codes.push_back(c3);
        } else if (c1.op == OP_FUNC) {
            cur_func = c1.num2;
            new_codes.push_back(c1);
        } else {
            new_codes.push_back(c1);
        }
    }
    codes = new_codes;
}

void PseudoCodeList::interpret() {
    //TODO: array
    vector<PseudoCode> new_codes;
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
        PseudoCode c = codes[i];
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
                    PseudoCodeList::strcons.push_back("@" + string(&ch));
                } else if (c.num2 == "char") {
                    char ch = (char) stoi(c.num1);
                    PseudoCodeList::strcons.push_back("@" + string(&ch));
                } else if (it1->valid && it1->dataType == integer) {
                    PseudoCodeList::strcons.push_back(it1->const_value);
                } else {
                    PseudoCodeList::strcons.push_back(c.num1);
                }
                new_codes.emplace_back(OP_PRINT, to_string(PseudoCodeList::strcons.size() - 1), "strcon", VACANT);
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
        } else if (is_arith(c.op)) {
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

void PseudoCodeList::divide_basic_blocks() {

}

bool is_arith(const string &op) {
    return (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV);
}
