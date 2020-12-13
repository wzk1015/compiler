//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_Pseudo_H
#define COMPILER_Pseudo_H

#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <fstream>
#include <map>
#include <cmath>

#include "utils.h"
#include "SymTable.h"

#define VACANT "#VACANT"
#define AUTO "#AUTO"
#define AUTO_VAR "#AUTO_VAR"
#define ENDL "#ENDL"
#define AUTO_LABEL "#AUTO_LABEL"
#define LOOP_BEGIN "LOOP BEGIN"
#define LOOP_END "LOOP END"

#define OP_PRINT "PRINT"
#define OP_SCANF "SCANF"
#define OP_ASSIGN ":="
#define OP_ADD "+"
#define OP_SUB "-"
#define OP_MUL "*"
#define OP_DIV "/"
#define OP_FUNC "FUNC"
#define OP_END_FUNC "END_FUNC"
#define OP_SRA ">>"

#define OP_ARR_LOAD "ARR_LOAD"
#define OP_ARR_SAVE "ARR_SAVE"
#define OP_LABEL "LABEL"
#define OP_JUMP_IF "JUMP_IF"
#define OP_JUMP_UNCOND "JUMP"

#define OP_PREPARE_CALL "PREPARE_CALL"
#define OP_CALL "CALL"
#define OP_PUSH_PARA "PUSH_PARA"
#define OP_RETURN "RETURN"

#define OP_EMPTY "EMPTY"
#define OP_PLACEHOLDER "PLACEHOLDER"


using namespace std;

class PseudoCode {
public:
    string op;
    string num1;
    string num2;
    string result;
    string comment;
    string info;

    PseudoCode(string op, string n1, string n2, string r) :
            op(std::move(op)), num1(std::move(n1)), num2(std::move(n2)), result(std::move(r)) {};

    PseudoCode() = default;

    string to_str() const;

    string to_standard_format() const;
};

class BasicBlock {
public:
    int index;
    int start;
    int end;
    vector<PseudoCode> codes;
    vector<BasicBlock> prev;
    vector<BasicBlock> next;

    BasicBlock(int index, int start, int end) : index(index), start(start), end(end) {}
};

class DAGNode {
public:
    int index;
    string name;
    bool is_leaf;
    bool in_queue = false;
    string primary_symbol;
    vector<string> symbols;
    vector<int> children;
    vector<int> parents;

    DAGNode(int index, const string& name, bool is_leaf) : index(index), name(name), is_leaf(is_leaf) {
        if (is_leaf) {
            symbols.push_back(name);
        }
    }
};

class PseudoCodeList {
public:
    static vector<PseudoCode> codes;
    static int code_index;
    static int label_index;
    static vector<string> strcons;
    static map<string, vector<BasicBlock>> blocks;
    static vector<DAGNode> DAGNodes;
    static map<string, int> NodesMap;
    static int call_times;

    static void reset() {
        codes.clear();
        strcons.clear();
        blocks.clear();
        code_index = 1;
        label_index = 1;
        DAGNodes.clear();
        NodesMap.clear();
    }

    static string add(const string &op, const string &n1, const string &n2, const string &r) {
        string result = r;
        if (result == AUTO) {
            result = "#T" + to_string(code_index);
            code_index++;
        } else if (result == AUTO_LABEL) {
            result = assign_label();
        } else if (result == AUTO_VAR) {
            result = "@V" + to_string(code_index);
            code_index++;
        }
        codes.emplace_back(op, n1, n2, result);
        return result;
    }

    static string assign_label() {
        label_index++;
        return "label_" + to_string(label_index - 1);
    }

    static bool var_modified_in_function(const string &var_name, const string &func);

    static void refactor();

    static void remove_redundant_tmp();

    static void remove_redundant_assign();

    static void remove_tripple();

    static void const_broadcast();

    static void divide_basic_blocks();

    static void dfs_show(const DAGNode &node, int depth, const string& mode, vector<string> vars=vector<string>());

    static void show_DAG_tree();

    static void DAG_optimize();

    static vector<PseudoCode> DAG_output();

    static void gen_DAG_graph(int, int);

    static void inline_function();

    static bool remove_shared_expr();

    static void loop_invariant_optimize();

    static void loop_var_pow2();

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
        for (int i = 0; i < codes.size(); i++) {
            //out << i << ": " << codes[i].to_str() << endl;
            if (!codes[i].comment.empty()) {
                out << "----------" + codes[i].comment + "----------" << endl;
            }
            out << codes[i].to_str() << endl;
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

bool is_arith(const string &op);

bool can_dag(const string &op);

string rename_inline_var(string name, vector<pair<DataType, string>> call_paras,
        vector<string> real_paras, int call_times, const string& call_func, const string& cur_func);

#endif //COMPILER_Pseudo_H
