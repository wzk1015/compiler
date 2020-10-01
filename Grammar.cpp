#include "Grammar.h"

vector<GrammarResults> Grammar::analyze(const char *out_path) {
    ofstream out;
    if (save_to_file) {
        out.open(out_path);
    }

    try {
        next_sym();
        Program();
    } catch (exception) {
        Errors::add("unexpected end of tokens", E_UNEXPECTED_EOF);
    }

    if (pos != tokens.size()) {
        Errors::add("unexpected extra token '" + tokens[pos].str + "' at end of file",
                    tokens[pos].line, tokens[pos].column, E_UNEXPECTED_CHAR);
    }

    if (save_to_file) {
        for (auto &str: output_str) {
            out << str << endl;
        }
    }

    out.close();

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
    if (pos >= tokens.size()) {
        throw exception();
    }
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
    tk = tokens[pos - 1];
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
}

void Grammar::error(const string &expected) {
    Errors::add("Expected " + expected + ", but got '" + tk.str + "' (type: " + sym + ")",
                tk.line, tk.column, E_GRAMMAR);
    while (sym != "SEMICN") {
        next_sym();    //skip until next statement
    }
    if (expected != "';'") {
        retract();
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
        } else {
            retract();
            retract();
            while (sym == "INTTK" || sym == "CHARTK" || sym == "VOIDTK") {
                if (sym == "INTTK" || sym == "CHARTK") {
                    RetFuncDef();
                } else {
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
    if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        Int();
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
        if (sym == "LPARENT") {  //function
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
    if (sym == "ASSIGN") {  //int a=1;
        next_sym();
        Const();
        init = true;
    } else if (sym == "LBRACK") {
        next_sym();
        UnsignedInt();
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        next_sym();
        if (sym == "ASSIGN") {  //int a[3]={1,2,3}
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
        } else if (sym == "LBRACK") {
            next_sym();
            UnsignedInt();
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
            if (sym == "ASSIGN") { //int a[3][3]={{1,2,3}, {4,5,6}, {7,8,9}}
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
                    next_sym();
                } while (sym == "COMMA");
                //已预读
                if (sym != "RBRACE") {
                    error("'}'");
                }
                init = true;
            } else {    //int a[3][3];
                retract();
                init = false;
            }
        } else {  //int a[3];
            retract();
            init = false;
        }
    } else {  //int a;
        retract();
        init = false;
    }

    if (init) {
        output("<变量定义及初始化>");
    } else {  //int a,b,c;
        next_sym();
        while (sym == "COMMA") {
            next_sym();
            Identifier();
            next_sym();
            if (sym == "LBRACK") {
                next_sym();
                UnsignedInt();
                next_sym();
                if (sym != "RBRACK") {
                    error("']'");
                }
                next_sym();
                if (sym == "LBRACK") {
                    next_sym();
                    UnsignedInt();
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
            next_sym();
        }
        retract();
        output("<变量定义无初始化>");
    }

    output("<变量定义>");
}


void Grammar::TypeIdentifier() {
    if (sym != "INTTK" && sym != "CHARTK") {
        error("'int' or 'char'");
    }
}

void Grammar::SharedFuncDef() {
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    if (sym == "RPARENT") {
        retract();
        output("<参数表>");
    } else {
        ParaList();
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
}

void Grammar::RetFuncDef() {
    DeclareHead();

    SymTableItem sti;
    sti.has_return = true;
    symTable.emplace(tk.str, sti);

    SharedFuncDef();

    output("<有返回值函数定义>");
}

void Grammar::NonRetFuncDef() {
    if (sym != "VOIDTK") {
        error("'void");
    }
    next_sym();
    Identifier();

    SymTableItem sti;
    sti.has_return = false;
    symTable.emplace(tk.str, sti);

    SharedFuncDef();

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
        } else if (sym == "LBRACK") {
            next_sym();
            Expr();
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
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
    } else if (sym == "IFTK") {
        ConditionStmt();
    } else if (sym == "SWITCHTK") {
        CaseStmt();
    } else if (sym == "SCANFTK") {
        ReadStmt();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    } else if (sym == "PRINTFTK") {
        WriteStmt();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    } else if (sym == "RETURNTK") {
        ReturnStmt();
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    } else if (sym == "LBRACE") {
        next_sym();
        StmtList();
        next_sym();
        if (sym != "RBRACE") {
            error("'}'");
        }
    } else if (sym == "SEMICN") {
        //空语句
    } else if (sym == "IDENFR") {
        next_sym(); // func(
        if (sym == "LPARENT") {
            retract(); //func
            if (symTable.find(tk.str) == symTable.end()) {
                Errors::add("undefined function call", tk.line, tk.column, E_UNDEFINED_IDENTF);
            }
            if (symTable.find(tk.str)->second.has_return) {
                RetFuncCall();
            } else {
                NonRetFuncCall();
            }
        } else if (sym == "ASSIGN" || sym == "LBRACK") {
            retract();
            AssignStmt();
        } else {
            error("assign statement or function call");
        }
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
    } else {
        error("statement");
    }
    output("<语句>");

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
        next_sym();
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
    if (sym != "COLON") {
        error("':'");
    }
    next_sym();
    Stmt();
    output("<情况子语句>");
}

void Grammar::Default() {
    if (sym != "DEFAULTTK") {
        error("'default'");
    }
    next_sym();
    if (sym != "COLON") {
        error("':'");
    }
    next_sym();
    Stmt();
    output("<缺省>");
}

void Grammar::SharedFuncCall() {
    Identifier();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    if (sym == "RPARENT") {
        retract();
        output("<值参数表>");
    } else {
        ValueParaList();
    }
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
}

void Grammar::RetFuncCall() {
    SharedFuncCall();
    output("<有返回值函数调用语句>");
}

void Grammar::NonRetFuncCall() {
    SharedFuncCall();
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
    while (sym == "WHILETK" || sym == "FORTK" || sym == "SWITCHTK" ||
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






