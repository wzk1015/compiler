#pragma once

#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include "Lexer.h"
#include "Error.h"
#include "SymTable.h"
#include "PseudoCode.h"

class TreeNode {
public:
    string str;
    string type;
    int parent;
    vector<int> child;

    TreeNode(string name, string type, int parent) : str(std::move(name)), type(std::move(type)), parent(parent) {}
};

enum GrammarMode {
    grammar_check,
    gen_inline,
    semantic_analyze,
};

/*
 * 第一遍grammar check：
 * 分析语法，检查错误
 * 记录函数体及是否能inline（静态变量function_tokens_index）
 * 保存至新文件
 * 第二遍gen inline:
 * 有返回值函数开头增加"&RET&"声明
 * 对于能inline的函数，直接替换为函数体（以复合语句形式），其中对参数进行替换（借助符号表）；
 * 保存至新文件
 * 第三遍semantic analyze：
 * 根据生成中间代码、建立符号表（实际上为了错误检查，前两遍也有此步骤，但已经通过reset清除）
 * 针对inline替换后的代码，此时的文法允许语句列内出现复合语句
 *
 * 若不开启inline，则只进行第一遍grammar check
*/

class FunctionIndex {
public:
    int begin;
    int end;
    string name;

    FunctionIndex(int begin, int end, string name) : begin(begin), end(end), name(std::move(name)) {}
};

class Grammar {
public:
    GrammarMode mode;
    Lexer lexer;
    vector<string> output_str;
    vector<Token> cur_lex_results;
    vector<Token> new_lex_results;
    int pos = 0;

    Token tk{INVALID};
    string sym = "";
    int local_addr = LOCAL_ADDR_INIT;

    vector<pair<DataType, string>> tmp_paras;
    int tmp_dim1{};
    int tmp_dim2{};
    int tmp_para_count = 0;

    DataType funcdef_ret = invalid_dt;
    bool has_returned = false;
    string cur_func = GLOBAL;
    int func_count = 0;

    vector<TreeNode> nodes;
    unsigned int cur_node = 0;

    static vector<FunctionIndex> function_tokens_index;
    int function_call_start_index = -1;
    int statement_begin_index = -1;
    int ret_index = 0;
    bool para_assigned = false;
    bool in_factor = false;

    void error(const string &expected);

    int next_sym(bool);

    void retract();

    int analyze();

    explicit Grammar(const string &in_path, GrammarMode mode) :
            lexer(in_path, mode != grammar_check), mode(mode) {};

    string add_midcode(const string &op, const string &n1, const string &n2, const string &r) const;

    string const_replace(string symbol) const;

    void output(const string &name);

    void save_to_file(const string &out_path);

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

    string add_sub(const string &num1, const string &num2);

    string add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r);

    string
    add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r,
                 int dim2_size);

    void add_node(const string &name);

    void add_leaf();

    void tree_backward();

    void dfs_show(const TreeNode &, int);

    void show_tree();

    void save_lexer_results(const string &path);


    void change_name();
};



