#pragma once

#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include "Token.h"
#include "Lexer.h"
#include "Error.h"
#include "SymTable.h"
#include "MidCode.h"

class Grammar {
public:
    Lexer lexer;
    vector<string> output_str;
    vector<Token> cur_lex_results;
    int pos = 0;
    string out_path = INVALID;

    Token tk{INVALID};
    string sym = "";
    int local_addr = 0;

    DataType tmp_const_data_type = invalid;
    DataType tmp_switch_data_type = invalid;
    vector<DataType> tmp_para_types;
    int tmp_dim1{};
    int tmp_dim2{};
    int tmp_para_count = 0;

    DataType funcdef_ret = invalid;
    bool has_returned = false;
    string cur_func = GLOBAL;

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

    string Const();

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

    void Condition();

    void LoopStmt();

    void PaceLength();

    void CaseStmt();

    void CaseList();

    void CaseSubStmt();

    void Default();

    void SharedFuncCall();

    void RetFuncCall();

    void NonRetFuncCall();

    void StmtList();

    void ReadStmt();

    void WriteStmt();

    void ReturnStmt();
};



