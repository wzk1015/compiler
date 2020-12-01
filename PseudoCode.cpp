//
// Created by wzk on 2020/11/5.
//

#include "PseudoCode.h"

vector<PseudoCode> PseudoCodeList::codes;
int PseudoCodeList::code_index = 1;
vector<string> PseudoCodeList::strcons;
int PseudoCodeList::label_index = 1;
map<string, vector<BasicBlock>> PseudoCodeList::blocks;
vector<DAGNode> PseudoCodeList::DAGNodes;
map<string, int> PseudoCodeList::NodesMap;
int PseudoCodeList::call_times = 0;


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
    for (auto &c : codes) {
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

        if (c.op == OP_ARR_SAVE && c.result == "#T33") {
            cout << (c.op == OP_ARR_SAVE) << endl;
            cout << begins_num(c.result) << endl;
            cout << !itr->const_value.empty() << endl;
        }

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

        } else if (c.op == OP_ARR_SAVE && (begins_num(c.result) || !itr->const_value.empty())) {
            int vr = stoi(begins_num(c.result) ? c.result : itr->const_value);
            if (num2_can_cal) {
                int v2 = stoi(begins_num(c.num2) ? c.num2 : it2->const_value);
                new_codes.emplace_back(c.op, c.num1, to_string(v2), to_string(vr));
            } else {
                new_codes.emplace_back(c.op, c.num1, c.num2, to_string(vr));
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
    map<string, set<int>> basic_block_idx;
    string cur_func = GLOBAL;
    for (int i = 0; i < codes.size(); i++) {
        string op = codes[i].op;
        PseudoCode c = codes[i];
        if (op == OP_FUNC) {
            cur_func = c.num2;
            basic_block_idx[cur_func] = set<int>();
            //函数第一条语句为入口语句
            for (int j = i + 1; codes[j].op != OP_END_FUNC; j++) {
                if (codes[j + 1].op == OP_END_FUNC) {
                    basic_block_idx[cur_func].insert(j + 1);
                }
                if (codes[j].op != OP_LABEL) {
                    basic_block_idx[cur_func].insert(j);
                    break;
                }
            }
        } else if (cur_func == GLOBAL) {
            //不给全局变量赋值语句划分基本块
            continue;
        }

        if (op == OP_LABEL || op == OP_JUMP_IF || op == OP_JUMP_UNCOND
            || op == OP_RETURN || op == OP_CALL) {
            for (int j = i + 1; codes[j].op != OP_END_FUNC; j++) {
                if (codes[j + 1].op == OP_END_FUNC) {
                    basic_block_idx[cur_func].insert(j + 1);
                }
                if (codes[j].op != OP_LABEL) {
                    basic_block_idx[cur_func].insert(j);
                    break;
                }
            }
        }
        if (op == OP_END_FUNC) {
            basic_block_idx[cur_func].insert(i);
        }
    }


    int index = 0;
    for (auto &it: basic_block_idx) {
        blocks[it.first] = vector<BasicBlock>();
        vector<int> vec(basic_block_idx[it.first].begin(), basic_block_idx[it.first].end());
        for (int i = 0; i < vec.size() - 1; i++) {
            blocks[it.first].emplace_back(index++, vec[i], vec[i + 1] - 1);
        }
    }

    cout << "=====basic blocks:=====" << endl;
    for (auto &it: blocks) {
        cout << "===function " << it.first << "===" << endl;
        for (auto &b: blocks[it.first]) {
            cout << "#" << b.index << " " << b.start << "~" << b.end << endl;
        }
    }

    //TODO：生成流图
}

void PseudoCodeList::gen_DAG_graph(int begin, int end) {
    for (int idx = begin; idx <= end; idx++) {
        PseudoCode c = codes[idx];
        string num1 = c.op == OP_ARR_SAVE ? c.num2 : c.num1;
        string num2 = c.op == OP_ARR_SAVE ? c.result : c.num2;
        string result = c.op == OP_ARR_SAVE ? c.num1 : c.result;
        if (c.op == OP_ASSIGN) {
            unsigned int k = -1;
            for (auto &n: NodesMap) {
                if (n.first == num2) {
                    k = n.second;
                    break;
                }
            }
            if (k == -1) {
                k = DAGNodes.size();
                string name = num2[0] == '#' ? num2 : num2 + "";
                DAGNodes.emplace_back(k, name, true);
                NodesMap[num2] = k;
            }
            DAGNodes[k].symbols.push_back(num1);

            bool find = false;
            for (auto &n: NodesMap) {
                if (n.first == num1) {
                    n.second = k;
                    find = true;
                    break;
                }
            }
            if (!find) {
                NodesMap[num1] = k;
            }

            continue;
        }

        unsigned int i = -1;
        for (auto &n: NodesMap) {
            if (n.first == num1) {
                i = n.second;
                break;
            }
        }
        if (i == -1) {
            i = DAGNodes.size();
            string name = num2[0] == '#' ? num1 : num1 + "";
            DAGNodes.emplace_back(i, name, true);
            NodesMap[num1] = i;
        }

        unsigned int j = -1;
        for (auto &n: NodesMap) {
            if (n.first == num2) {
                j = n.second;
                break;
            }
        }
        if (j == -1) {
            j = DAGNodes.size();
            string name = num2[0] == '#' ? num2 : num2 + "";
            DAGNodes.emplace_back(j, name, true);
            NodesMap[num2] = j;
        }

        unsigned int k = -1;
        for (auto &n: DAGNodes) {
            if (n.name == c.op && n.children[0] == 0 && n.children[1] == j) {
                k = n.index;
                break;
            }
        }
        if (k == -1) {
            k = DAGNodes.size();
            DAGNode new_node(DAGNodes.size(), c.op, false);
            new_node.children.push_back(i);
            new_node.children.push_back(j);
            DAGNodes[i].parents.push_back(k);
            DAGNodes[j].parents.push_back(k);
            DAGNodes.push_back(new_node);
        }
        DAGNodes[k].symbols.push_back(result);

        bool find = false;
        for (auto &n1: NodesMap) {
            if (n1.first == result) {
                n1.second = k;
                find = true;
                break;
            }
        }
        if (!find) {
            NodesMap[result] = k;
        }
    }

    for (auto &n: DAGNodes) {
        bool has_find = false;
        for (auto &name: n.symbols) {
            if (name[0] != '#') {
                n.primary_symbol = name;
                has_find = true;
                break;
            }
        }
        if (!has_find) {
            n.primary_symbol = n.symbols[0];
        }
    }
}

vector<PseudoCode> PseudoCodeList::DAG_output() {
    vector<PseudoCode> ret;
    vector<DAGNode> queue;
    cout << "total number of DAGNodes: " << DAGNodes.size() << endl;
    while (true) {
        bool break_flag = true;
        for (auto &n: DAGNodes) {
            if (!n.is_leaf && !n.in_queue) {
                break_flag = false;
                //cout << "dag output loop1" << endl;
            }
        }
        if (break_flag) {
            break;
        }

        int i;
        for (i = 0; i < DAGNodes.size(); i++) {
            if (DAGNodes[i].parents.empty() && !DAGNodes[i].is_leaf && !DAGNodes[i].in_queue) {
                queue.push_back(DAGNodes[i]);
                DAGNodes[i].in_queue = true;
                cout << "Node" << i << "enqueue" << endl;
                break;
            }
        }
        for (auto &node: DAGNodes) {
            vector<int> new_parents;
            for (auto &parent: node.parents) {
                if (parent != i) {
                    new_parents.push_back(parent);
                }
            }
            node.parents = new_parents;
        }
        if (!DAGNodes[i].children.empty()) {
            int child_id = DAGNodes[i].children[0];
            DAGNode cur = DAGNodes[child_id];
            while (cur.parents.empty() && !cur.is_leaf && !DAGNodes[i].in_queue) {
                queue.push_back(cur);
                cur.in_queue = true;
                for (auto &node: DAGNodes) {
                    vector<int> new_parents;
                    for (auto &parent: node.parents) {
                        if (parent != child_id) {
                            new_parents.push_back(parent);
                        }
                    }
                    node.parents = new_parents;
                }
                if (!cur.children.empty()) {
                    child_id = cur.children[0];
                    cur = DAGNodes[child_id];
                } else {
                    break;
                }
            }
        }
    }

    for (auto &n : DAGNodes) {
        if (n.is_leaf) {
            for (auto &name: n.symbols) {
                if (name[0] != '#' && name != n.primary_symbol && NodesMap[name] == n.index) {
                    ret.emplace_back(OP_ASSIGN, name, n.primary_symbol, VACANT);
                }
            }
        }
    }

    for (int i = (int) queue.size() - 1; i >= 0; i--) {
//        cout << "queue[" << i << "]: " << queue[i].primary_symbol << endl;
        assertion(can_dag(queue[i].name));
        bool has_print = false;
        for (auto &name: queue[i].symbols) {
            if (name[0] != '#') {
                if (queue[i].name == OP_ARR_SAVE) {
                    ret.emplace_back(queue[i].name, name, DAGNodes[queue[i].children[0]].primary_symbol,
                                     DAGNodes[queue[i].children[1]].primary_symbol);
                } else {
                    ret.emplace_back(queue[i].name, DAGNodes[queue[i].children[0]].primary_symbol,
                                     DAGNodes[queue[i].children[1]].primary_symbol, name);
                }
                has_print = true;
            }
        }
        if (!has_print) {
            if (queue[i].name == OP_ARR_SAVE) {
                ret.emplace_back(queue[i].name, queue[i].primary_symbol, DAGNodes[queue[i].children[0]].primary_symbol,
                                 DAGNodes[queue[i].children[1]].primary_symbol);
            } else {
                ret.emplace_back(queue[i].name, DAGNodes[queue[i].children[0]].primary_symbol,
                                 DAGNodes[queue[i].children[1]].primary_symbol, queue[i].primary_symbol);
            }
        }
    }

    cout << "AFTER DAG" << endl;
    for (auto &code: ret) {
        cout << code.to_str() << endl;
    }
    return ret;
}

void PseudoCodeList::DAG_optimize() {
    for (auto &it: blocks) {
        for (auto &b: blocks[it.first]) {
            for (int i = b.start; i < b.end; i++) {
                if (!can_dag(codes[i].op)) continue;
                cout << "BEFORE DAG" << endl;
                cout << codes[i].to_str() << endl;
                int j = i + 1;
                while (can_dag(codes[j].op)) {
                    cout << codes[j].to_str() << endl;
                    j++;
                }
                if (j - i <= 1) continue;

                cout << "DAG for " << i << "~" << j - 1 << endl;
                gen_DAG_graph(i, j - 1);
                cout << "gen DAG graph done" << endl;
                show_DAG_tree();
                if (!DAGNodes.empty()) {
                    vector<PseudoCode> dag_outputs = DAG_output();
                    assertion(dag_outputs.size() <= j - i);
                    for (int m = i; m <= j - 1; m++) {
                        if (m - i < dag_outputs.size()) {
                            codes[m] = dag_outputs[m - i];
                        } else {
                            codes[m] = PseudoCode(OP_PLACEHOLDER, VACANT, VACANT, VACANT);
                        }
                    }
                    //TODO: replace old with new
                }
                i = j;

                DAGNodes.clear();
                NodesMap.clear();
            }
        }
    }

    vector<PseudoCode> new_codes;
    for (auto &c: codes) {
        if (c.op != OP_PLACEHOLDER) {
            new_codes.push_back(c);
        }
    }
    codes = new_codes;
}

void PseudoCodeList::dfs_show(const DAGNode &node, int depth) {
    for (int i = 0; i < depth - 1; i++) {
        cout << "|     ";
    }
    if (depth != 0) {
        cout << "|-----";
    }
    cout << "'" << node.name << "' " << node.primary_symbol << "(" << node.index << ") [";
    for (auto &s: node.symbols) {
        if (s != node.primary_symbol) {
            cout << s << ",";
        }
    }
    cout << "]" << (node.is_leaf ? " LEAF" : "") << endl;
    for (auto &child: node.children) {
        dfs_show(DAGNodes[child], depth + 1);
    }
}

void PseudoCodeList::show_DAG_tree() {
    DAGNode root(-1, "ROOT", false);
    for (auto &node: DAGNodes) {
        if (node.parents.empty()) {
            root.children.push_back(node.index);
        }
    }
    dfs_show(root, 0);
//    for (auto &it: NodesMap) {
//        cout << it.first << ": " << it.second << endl;
//    }
}

void PseudoCodeList::inline_function() {
    vector<PseudoCode> new_codes;
    map<string, vector<PseudoCode>> func_codes;
    string cur_func = INVALID;
    for (auto &code: codes) {
        if (code.op == OP_FUNC) {
            cur_func = code.num2;
            func_codes[cur_func] = vector<PseudoCode>();
        }
        else if (code.op == OP_END_FUNC) {
            cur_func = INVALID;
        }
        else if (cur_func != INVALID) {
            func_codes[cur_func].push_back(code);
        }
    }

    vector<vector<string>> paras;
    vector<string> call_func;
    cur_func = GLOBAL;
    for (auto &code: codes) {
        if (code.op == OP_FUNC) {
            cur_func = code.num2;
        }

        if (code.op == OP_PREPARE_CALL) {
            if (SymTable::search_func(code.num1).recur_func) {
                new_codes.push_back(code);
                continue;
            }
            paras.emplace_back();
            call_func.push_back(code.num1);
            call_times++;
            cout << "inline func: " << call_func.back() << endl;
        } else if (code.op == OP_PUSH_PARA && !call_func.empty()) {
            if (code.num1[0] == '#') {
                string var_name = "@V" + to_string(code_index);
                code_index++;
                new_codes.emplace_back(OP_ASSIGN, var_name, code.num1, VACANT);
                int addr = SymTable::func_size(cur_func) + 4;
                SymTable::add(cur_func, var_name, var, SymTable::try_search(cur_func, code.num1, true).dataType, addr);
                paras.back().push_back(var_name);
            } else {
                paras.back().push_back(code.num1);
            }
        } else if (code.op == OP_CALL && !call_func.empty()) {
            assertion(!call_func.empty());
            vector<pair<DataType, string>> call_paras = SymTable::search_func(call_func.back()).paras;
            string label_end_func = assign_label();
            vector<PseudoCode> call_codes = func_codes[call_func.back()];
            for (int i = 0; i < call_codes.size(); i++) {
                PseudoCode c = call_codes[i];
                string result = rename_inline_var(c.result, call_paras, paras.back(), call_times, call_func.back(), cur_func);
                string num1 = rename_inline_var(c.num1, call_paras, paras.back(), call_times, call_func.back(), cur_func);
                string num2 = rename_inline_var(c.num2, call_paras, paras.back(), call_times, call_func.back(), cur_func);
                if (c.op == OP_RETURN) {
                    if (c.num1 != VACANT) {
                        new_codes.emplace_back(OP_ASSIGN, "%RET", num1, VACANT);
                    }
                    if (i == call_codes.size() - 1
                    || (c.op == OP_RETURN && call_codes[i+1].to_str() == c.to_str())) {

                    } else {
                        new_codes.emplace_back(OP_JUMP_UNCOND, label_end_func, VACANT, VACANT);
                    }
                } else {
                    new_codes.emplace_back(c.op, num1, num2, result);
                }
            }

            new_codes.emplace_back(OP_LABEL, label_end_func, VACANT, VACANT);
            call_func.pop_back();
            paras.pop_back();
        } else  { // if (call_func.empty())
            new_codes.push_back(code);
        }
    }

    codes = new_codes;
}

string rename_inline_var(string name, vector<pair<DataType, string>> call_paras,
        vector<string> real_paras, int call_times, const string& call_func, const string& cur_func) {
    for (int i = 0; i < call_paras.size(); i++) {
        if (call_paras[i].second == name) {
            cout << "replace " << name << " with " << real_paras[i] << endl;
            return real_paras[i];
        }
    }
    if (begins_num(name) || SymTable::in_global(call_func, name)) {
        return name;
    }
    SymTableItem item = SymTable::try_search(call_func, name, false);
    if (item.valid) {
        string ret =  "_" + to_string(call_times) + "_" + name;
        if (!SymTable::try_search(cur_func, ret, false).valid) {
            int addr = SymTable::func_size(cur_func) + 4;
            SymTable::add(cur_func, ret, item.stiType, item.dataType, addr, item.dim1_size, item.dim2_size);
        }
        return ret;
    }
    else if (name.substr(0,5)=="label") {
        return  name + "_" + to_string(call_times) + "_";
    }

    return name;
}

bool is_arith(const string &op) {
    return (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV);
}

bool can_dag(const string &op) {
    return is_arith(op) || op == OP_ASSIGN ; //|| op == OP_ARR_LOAD || op == OP_ARR_SAVE;
}