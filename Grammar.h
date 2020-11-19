#pragma once

#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include "Lexer.h"
#include "Error.h"
#include "SymTable.h"
#include "MidCode.h"

class TreeNode {
public:
    string str;
    string type;
    int parent;
    vector<int> child;

    TreeNode(string name, string type, int parent): str(std::move(name)), type(std::move(type)), parent(parent) {}
};

class Grammar {
public:
    Lexer lexer;
    vector<string> output_str;
    vector<Token> cur_lex_results;
    int pos = 0;
    string out_path = INVALID;

    Token tk{INVALID};
    string sym = "";
    int local_addr = LOCAL_ADDR_INIT;

    vector<pair<DataType, string>> tmp_paras;
    int tmp_dim1{};
    int tmp_dim2{};
    int tmp_para_count = 0;

    DataType funcdef_ret = invalid;
    bool has_returned = false;
    string cur_func = GLOBAL;

    vector<TreeNode> nodes;
    unsigned int cur_node = 0;

    void error(const string &expected);

    int next_sym();

    void retract();

    int analyze();

    Grammar(const string &in_path, string out_path) : out_path(std::move(out_path)), lexer(in_path) {};

    explicit Grammar(const string &in_path) : lexer(in_path) {};

    string add_midcode(const string &op, const string &n1, const string &n2, const string &r) const;

    string const_replace(string symbol) const;

    void output(const string &name);

    void Program();

    void ConstDeclare();

    void ConstDef();

    void UnsignedInt();

    string Int();

    string Identifier();

    pair<DataType, string> Const();

    void VariableDeclare();

    void VariableDef();;

    void TypeIdentifier();

    void SharedFuncDefHead();

    void SharedFuncDefBody();

    void RetFuncDef();

    void NonRetFuncDef();

    void CompoundStmt();

    void ParaList();

    void Main();

    pair<DataType, string> Expr();

    pair<DataType, string> Item();

    pair<DataType, string> Factor();

    void Stmt();

    void AssignStmt();

    void ConditionStmt();

    pair<string, string> Condition();

    void LoopStmt();

    void CaseStmt();

    void SharedFuncCall();

    void RetFuncCall();

    void NonRetFuncCall();

    void StmtList();

    void ReadStmt();

    void WriteStmt();

    void ReturnStmt();

    string add_sub(const string& num1, const string& num2);

    string add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r);

    string
    add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r,
                 int dim2_size);

    void add_node(const string& name);

    void add_leaf();

    void tree_backward();

    void dfs_show(const TreeNode&, int);

    void show_tree();
};



