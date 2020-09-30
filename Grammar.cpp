#include "Grammar.h"

vector<Error> err;

vector<GrammarResults> Grammar::analyze(const char *out_path) {
    ofstream out;
    if (save_to_file) {
        out.open(out_path);
    }

    next_sym();
    Program();

    if (save_to_file) {
        for (auto &str: output_str) {
            out << str << endl;
        }
    }

    out.close();
    errors.insert(errors.begin(), err.begin(), err.end());

    cout << "Grammar complete successfully." << endl;
    return vector<GrammarResults>();
}

void Grammar::output(const string &str) {
    output_str.push_back(str);
    if (DEBUG) {
        cout << str << endl;
    }
}

int Grammar::next_sym() {
    tk = tokens[pos++];
    sym = tk.type;
//    str = tk.str;
    output_str.push_back(tk.type + " " + tk.str);
    if (DEBUG) {
        cout << tk.type + " " + tk.str << endl;
    }
    return 0;
}

void Grammar::retract() {
    pos--;
    tk = tokens[pos-1];
    sym = tk.type;
    for (unsigned int i = output_str.size() - 1; i >= 0; i--) {
        if (output_str[i][0] != '<') {
            if (DEBUG) {
                cout << "retract " << output_str[i] << endl;
            }
            output_str.erase(output_str.begin() + i);
            break;
        }
    }
    //output_str.pop_back();
}

void Grammar::error(const string &expected) {
    err.emplace_back("Expected " + expected + ", but got " + tk.str + " (type: " + sym + ")",
                         tk.line, tk.column, E_GRAMMAR);
    if (DEBUG) {
        cout << err[err.size()-1].msg << endl;
    }
}


void Grammar::Program() {
    if (sym == "CONSTTK") {
        ConstDeclare();
        next_sym();
    }
    while (sym == "INTTK" || sym == "CHARTK" || sym == "VOIDTK") {
        next_sym(); //int a
        next_sym(); //int a(  /  int a[  /  int a =
        if (sym != "LPARENT") {
            retract();
            retract();
            VariableDeclare();
        }
        else {
            retract();
            retract();
            while (sym == "INTTK" || sym == "CHARTK" || sym == "VOIDTK") {
                if (sym == "INTTK" || sym == "CHARTK") {
                    RetFuncDef();
                } 
                else {
                    next_sym();
                    if (sym == "MAINTK") {
                        retract();
                        Main();
                        output("<程序>");
                        return;
                    } else {
                        retract();
                        NonRetFuncDef();
                    }
                }
                next_sym();
                retract();
            }
        }
        next_sym();
    }

}

void Grammar::ConstDeclare() {
    do {
        if (sym != "CONSTTK") {
            error("'const'");
        }
        next_sym();
        ConstDef();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        next_sym();
    } while (sym == "CONSTTK");

    output("<常量说明>");

    retract();
}

void Grammar::ConstDef() {
    if (sym == "INTTK") {
        do {
            next_sym();
            Identifier();
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            Int();
            next_sym();
        } while (sym == "COMMA");
    } else if (sym == "CHARTK") {
        do {
            next_sym();
            Identifier();
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            if (sym != "CHARCON") {
                error("char");
            }
            next_sym();
        } while (sym == "COMMA");
    } else {
        error("const def");
    }
    output("<常量定义>");

    retract();
}

void Grammar::UnsignedInt() {
    if (sym != "INTCON") {
        error("unsigned int");
    }
    output("<无符号整数>");
}

void Grammar::Int() {
    if (sym == "PLUS" || sym == "MINU") {
        next_sym();
    }
    UnsignedInt();
    output("<整数>");
}

void Grammar::Identifier() {
    if (sym != "IDENFR") {
        error("identifier");
    }
}

void Grammar::DeclareHead() {
    if (sym != "INTTK" && sym != "CHARTK") {
        error("'int' or 'char'");
    }
    next_sym();
    Identifier();
    output("<声明头部>");
}

void Grammar::Const() {
    if (sym == "PLUS" || sym == "MINUS" || sym == "INTCON") {
        UnsignedInt();
    } else if (sym != "CHARCON") {
        error("char");
    }
    output("<常量>");
}

void Grammar::VariableDeclare() {
    VariableDef();
    next_sym();
    if (sym != "SEMICN") {
        error("';'");
    }
    next_sym();
    while (sym == "INTTK" || sym == "CHARTK") {
        next_sym(); //int a
        next_sym(); //int a(  /  int a;
        if (sym != "SEMICN") {  //function
            retract(); //int a
            retract(); //int
            //another retract at end of file
            break;
        }
        retract();
        retract();
        VariableDef();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        next_sym();
    }

    output("<变量说明>");

    retract();
}

void Grammar::VariableDef() {
    bool init;

    TypeIdentifier();
    next_sym();
    Identifier();
    next_sym();
    if (sym == "ASSIGN") {
        next_sym();
        Const();
        init = true;
    }
    else if (sym == "LBRACK") {
        next_sym();
        UnsignedInt();
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        next_sym();
        if (sym == "ASSIGN") {
            next_sym();
            if (sym != "LBRACE") {
                error("'{'");
            }
            do {
                next_sym();
                Const();
                next_sym();
            } while (sym == "COMMA");
            //已预读
            if (sym != "RBRACE") {
                error("'}'");
            }
            init = true;
        }
        else if (sym == "LBRACK") {
            next_sym();
            UnsignedInt();
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
            if (sym == "ASSIGN") {
                next_sym();
                if (sym != "LBRACE") {
                    error("'{'");
                }
                do {
                    next_sym();
                    if (sym != "LBRACE") {
                        error("'{'");
                    }
                    do {
                        next_sym();
                        Const();
                        next_sym();
                    } while (sym == "COMMA");
                    //已预读
                    if (sym != "RBRACE") {
                        error("'}'");
                    }
                } while (sym == "COMMA");
                //已预读
                if (sym != "RBRACE") {
                    error("'}'");
                }
                init = true;
            } else {
                retract();
                init = false;
            }
        }
        else {
            retract();
            init = false;
        }
    }
    else {
        retract();
        init = false;
    }

    if (init) {
        output("<变量定义及初始化>");
    }
    else {
        output("<变量定义无初始化>");
    }

    output("<变量定义>");
}


void Grammar::TypeIdentifier() {
    if (sym != "INTTK" && sym != "CHARTK") {
        error("'int' or 'char'");
    }
}

void Grammar::RetFuncDef() {
    DeclareHead();

    SymTableItem sti;
    sti.has_return = true;
    symTable.emplace(tk.str, sti);

    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    ParaList();
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    next_sym();
    if (sym != "LBRACE") {
        error("'{'");
    }
    next_sym();
    CompoundStmt();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }

    output("<有返回值函数定义>");
}

void Grammar::NonRetFuncDef() {
    if (sym != "VOIDTK") {
        error("'void");
    }
    Identifier();

    SymTableItem sti;
    sti.has_return = false;
    symTable.emplace(tk.str, sti);

    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    ParaList();
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    next_sym();
    if (sym != "LBRACE") {
        error("'{'");
    }
    next_sym();
    CompoundStmt();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    output("<无返回值函数定义>");
}

void Grammar::CompoundStmt() {
    if (sym == "CONSTTK") {
        ConstDeclare();
        next_sym();
    }
    if (sym == "INTTK" || sym == "CHARTK") {
        VariableDeclare();
        next_sym();
        //assert: 语句不以INTTK CHARTK开头
    }
    StmtList();
    output("<复合语句>");
}

void Grammar::ParaList() {
    TypeIdentifier();
    next_sym();
    Identifier();
    next_sym();
    while (sym == "COMMA") {
        next_sym();
        TypeIdentifier();
        next_sym();
        Identifier();
        next_sym();
    }

    output("<参数表>");

    retract();
    //空
}

void Grammar::Main() {
    if (sym != "VOIDTK") {
        error("'void'");
    }
    next_sym();
    if (sym != "MAINTK") {
        error("'main'");
    }
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    next_sym();
    if (sym != "LBRACE") {
        error("'{'");
    }
    next_sym();
    CompoundStmt();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    output("<主函数>");
}

void Grammar::Expr() {
    if (sym == "PLUS" || sym == "MINU") {
        next_sym();
    }
    Item();
    next_sym();
    while (sym == "PLUS" || sym == "MINU") {
        next_sym();
        Item();
        next_sym();
    }
    output("<表达式>");

    retract();
}

void Grammar::Item() {
    Factor();
    next_sym();
    while (sym == "MULT" || sym == "DIV") {
        next_sym();
        Factor();
        next_sym();
    }

    output("<项>");

    retract();
}

void Grammar::Factor() {
    if (sym == "IDENFR") {
        next_sym(); // func(
        if (sym == "LPARENT") {
            retract(); //func
            RetFuncCall();
        }
        else if (sym == "LBRACK") {
            next_sym();
            Expr();
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            if (sym == "LBRACK") {
                next_sym();
                Expr();
                next_sym();
                if (sym != "RBRACK") {
                    error("']'");
                }
            } else {
                retract();
            }
        } else {
            retract();
        }
    } else if (sym == "LPARENT") {
        next_sym();
        Expr();
        next_sym();
        if (sym != "RPARENT") {
            error(")");
        }
    } else if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        Int();
    } else if (sym == "CHARCON") {
        //
    } else {
        error("factor");
    }

    output("<因子>");
}

void Grammar::Stmt() {
    if (sym == "WHILETK" || sym == "FORTK") {
        LoopStmt();
    }
    else if (sym == "IFTK") {
        ConditionStmt();
    }
    else if (sym == "SWITCHTK") {
        CaseStmt();
    }
    else if (sym == "SCANFTK") {
        ReadStmt();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    }
    else if (sym == "PRINTFTK") {
        WriteStmt();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    }
    else if (sym == "RETURNTK") {
        ReturnStmt();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    }
    else if (sym == "LBRACE") {
        next_sym();
        StmtList();
        next_sym();
        if (sym != "RBRACE") {
            error("'}'");
        }
    }
    else if (sym == "SEMICN") {
        //空语句
    }
    else if (sym == "IDENFR") {
        next_sym(); // func(
        if (sym == "LPARENT") {
            retract(); //func
            if (symTable.find(tk.str) == symTable.end()) {
                //TODO：语义错误，未定义函数
                err.emplace_back("undefined function call", tk.line, tk.column, E_UNDEFINED_IDENTF);
            }
            if (symTable.find(tk.str)->second.has_return) {
                RetFuncCall();
            } else {
                NonRetFuncCall();
            }
        }
        else if (sym == "ASSIGN" || sym == "LBRACK") {
            retract();
            AssignStmt();
        }
        else {
            error("assign statement or function call");
        }
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    }
    else {
        error("statement");
    }
    output("<语句>");

    //空+';'
}

void Grammar::AssignStmt() {
    Identifier();
    next_sym();
    if (sym == "ASSIGN") {
        next_sym();
        Expr();
    } else if (sym == "LBRACK") {
        next_sym();
        Expr();
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        if (sym == "ASSIGN") {
            next_sym();
            Expr();
        } else if (sym == "LBRACK") {
            next_sym();
            Expr();
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            Expr();
        } else {
            error("array assign statement");
        }
    } else {
        error("assign statement");
    }

    output("<赋值语句>");
}

void Grammar::ConditionStmt() {
    if (sym != "IFTK") {
        error("'if'");
    }
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    Condition();
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    next_sym();
    Stmt();
    next_sym();
    if (sym == "ELSETK") {
        Stmt();
        next_sym();
    }
    output("<条件语句>");

    retract();
}

void Grammar::Condition() {
    Expr();
    next_sym();
    if (sym != "LSS" && sym != "LEQ" && sym != "GRE" && sym != "GEQ" && sym != "EQL" && sym != "NEQ") {
        error("relation operator");
    }
    next_sym();
    Expr();
    output("<条件>");
}

void Grammar::LoopStmt() {

    if (sym == "WHILETK") {
        next_sym();
        if (sym != "LPARENT") {
            error("(");
        }
        next_sym();
        Condition();
        next_sym();
        if (sym != "RPARENT") {
            error(")");
        }
        next_sym();
        Stmt();
    } else if (sym == "FORTK") {
        next_sym();
        if (sym != "LPARENT") {
            error("(");
        }
        next_sym();
        Identifier();
        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        next_sym();
        Expr();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        next_sym();

        Condition();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        next_sym();

        Identifier();
        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        next_sym();
        Identifier();
        next_sym();
        if (sym != "PLUS" && sym != "MINU") {
            error("'+' or '-'");
        }
        next_sym();
        PaceLength();
        next_sym();
        if (sym != "RPARENT") {
            error(")");
        }
        next_sym();
        Stmt();
    } else {
        error("'while' or 'for'");
    }


    output("<循环语句>");
}

void Grammar::PaceLength() {
    UnsignedInt();
    output("<步长>");
}

void Grammar::CaseStmt() {
    if (sym != "SWITCHTK") {
        error("'switch'");
    }
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    Expr();
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    next_sym();
    if (sym != "LBRACE") {
        error("'{'");
    }
    next_sym();
    CaseList();
    next_sym();
    Default();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }

    output("<情况语句>");
}

void Grammar::CaseList() {
    CaseSubStmt();
    next_sym();
    while (sym == "CASETK") {
        next_sym();
        CaseSubStmt();
        next_sym();
    }

    output("<情况表>");

    retract();
}

void Grammar::CaseSubStmt() {
    if (sym != "CASETK") {
        error("'case'");
    }
    next_sym();
    Const();
    next_sym();
    Stmt();
    output("<情况子语句>");
}

void Grammar::Default() {
    if (sym != "DEFAULTTK") {
        error("'default'");
    }
    next_sym();
    Stmt();
    output("<缺省>");
}

void Grammar::RetFuncCall() {
    Identifier();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    if (sym == "RPARENT") {
        output("值参数表");
    } else {
        ValueParaList();
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
    }

    output("<有返回值函数调用语句>");
}

void Grammar::NonRetFuncCall() {
    Identifier();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    if (sym == "RPARENT") {
        output("值参数表");
    } else {
        ValueParaList();
        if (sym != "RPARENT") {
            error("')'");
        }
    }

    output("<无返回值函数调用语句>");
}

void Grammar::ValueParaList() {
    Expr();
    next_sym();
    while (sym == "COMMA") {
        next_sym();
        Expr();
        next_sym();
    }

    output("<值参数表>");

    retract();
    //空
}

void Grammar::StmtList() {
    while (sym == "WHILETK" || sym == "DOTK" || sym == "FORTK" ||
           sym == "IFTK" || sym == "SCANFTK" || sym == "PRINTFTK" ||
           sym == "RETURNTK" || sym == "LBRACE" || sym == "IDENFR" ||
           sym == "SEMICN") {
        Stmt();
        next_sym();
    }
    output("<语句列>");
    retract();
    //空
}

void Grammar::ReadStmt() {
    if (sym != "SCANFTK") {
        error("'scanf'");
    }
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    Identifier();
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }

    output("<读语句>");
}

void Grammar::WriteStmt() {
    if (sym != "PRINTFTK") {
        error("'printf'");
    }
    next_sym();
    if (sym != "LPARENT") {
        error("'(");
    }
    next_sym();
    if (sym == "STRCON") {
        output("<字符串>");
        next_sym();
        if (sym == "COMMA") {
            next_sym();
            Expr();
            next_sym();
            if (sym != "RPARENT") {
                error("')'");
            }
        } else if (sym != "RPARENT") {
            error("')'");
        }
    } else {
        Expr();
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
    }
    output("<写语句>");
}

void Grammar::ReturnStmt() {
    if (sym != "RETURNTK") {
        error("'return'");
    }
    next_sym();
    if (sym == "LPARENT") {
        next_sym();
        Expr();
        next_sym();
        if (sym != "RPARENT") {
            error(")");
        }
        next_sym();
    }
    output("<返回语句>");

    retract();
}






