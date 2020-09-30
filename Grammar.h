#pragma once

#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include "LexResults.h"
#include "GrammarResults.h"
#include "error.h"
#include "SymTableItem.h"

class Grammar {
public:
    vector<LexResults> tokens;
    vector<Error> errors;
    map<string, SymTableItem> symTable;
    vector<string> output_str;

    bool save_to_file;

    LexResults tk{INVALID, INVALID, -1, -1, -1};
    int pos = 0;
    string sym = "";
//    string str = "";
    bool try_back = false;
    bool back = false;

    void error(const string &expected);

    int next_sym();

    void retract();

//    bool back_track(const string &func);

    vector<GrammarResults> analyze(const char *out_path);

    Grammar(vector<LexResults> t, bool save) : tokens(std::move(t)), save_to_file(save) {};

    void output(const string& name);

//    void AddOp();
//
//    void MulOp();
//
//    void RelationOp();
//
//    void Letter();
//
//    void Digit();

//    void Char();
//
//    void Str();

    void Program();

    void ConstDeclare();

    void ConstDef();

    void UnsignedInt();

    void Int();

    void Identifier();

    void DeclareHead();

    void Const();

    void VariableDeclare();

    void VariableDef();

//    void VariableDefNoInit();
//
//    void VariableDefInit();

    void TypeIdentifier();

    void RetFuncDef();

    void NonRetFuncDef();

    void CompoundStmt();

    void ParaList();

    void Main();

    void Expr();

    void Item();

    void Factor();

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

    void RetFuncCall();

    void NonRetFuncCall();

    void ValueParaList();

    void StmtList();

    void ReadStmt();

    void WriteStmt();

    void ReturnStmt();
};



