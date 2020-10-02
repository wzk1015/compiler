#pragma once

#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include "LexResults.h"
#include "Lexer.h"
#include "GrammarResults.h"
#include "Error.h"
#include "SymTableItem.h"

class Grammar {
public:
    Lexer lexer;
    map<string, SymTableItem> symTable;
    vector<string> output_str;
    vector<LexResults> cur_lex_results;
    int pos = 0;

    string out_path = INVALID;

    LexResults tk{INVALID};
    string sym = "";

    void error(const string &expected);

    int next_sym();

    void retract();


    vector<GrammarResults> analyze();

    Grammar(const string& in_path, string out_path) : out_path(std::move(out_path)), lexer(in_path){};

    explicit Grammar(const string& in_path) : lexer(in_path){};

    void output(const string &name);

    void Program();

    void ConstDeclare();

    void ConstDef();

    void UnsignedInt();

    void Int();

    void Identifier();

    void DeclareHead();

    void Const();

    void VariableDeclare();

    void VariableDef();;

    void TypeIdentifier();

    void SharedFuncDef();

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

    void SharedFuncCall();

    void RetFuncCall();

    void NonRetFuncCall();

    void ValueParaList();

    void StmtList();

    void ReadStmt();

    void WriteStmt();

    void ReturnStmt();
};



