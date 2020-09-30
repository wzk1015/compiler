#include "Grammar.h"
#include <fstream>

vector<Error> err;

vector<GrammarResults> Grammar::analyze(const char *out_path) {
    if (save_to_file) {
        out.open(out_path);
    }

    next_sym();
    Program();

    out.close();
    errors.insert(errors.begin(), err.begin(), err.end());
    return vector<GrammarResults>();
}

void Grammar::output(const string &name) {
    if (save_to_file) {
        out << name << endl;
    }
}

int Grammar::next_sym() {
    tk = tokens[pos++];
    sym = tk.type;
//    str = tk.str;
    return 0;
}

void Grammar::retract() {
    pos--;
    tk = tokens[pos];
    sym = tk.type;
}

void Grammar::error(const string &expected) {
    if (!try_back) {
        err.emplace_back("expected " + expected + " got " + tk.str + " (type: " + sym + ")",
                         tk.line, tk.column, E_GRAMMAR);
    } else {
        back = true;
    }
}

//bool Grammar::back_track(const string &func) {
//    int old_pos = pos;
//    try_back = true;
//    if (func == "UnsignedInt") {
//        UnsignedInt();
//    } else if (func == "VariableDeclare") {
//        VariableDeclare();
//    } else if (func == "VariableDefNoInit") {
//        VariableDefNoInit();
//    } else if (func == "VariableDefInit") {
//        VariableDefInit();
//    }
//    try_back = false;
//    if (back) {
//        pos = old_pos;
//        tk = tokens[pos - 1];
//        sym = tk.type;
//        back = false;
//        return true;
//    }
//    return false;
//}


//void Grammar::AddOp() {
//    if (sym != "PLUS" && sym != "MINU") {
//        error("'+' or '-'");
//    }
//}
//
//void Grammar::MulOp() {
//    if (str != "*" && str != "/") {
//        error("'*' or '/'");
//    }
//}

//void Grammar::RelationOp() {
//    if (str != "<=" && str != "<" && str != ">=" && str != ">" && str != "==" && str != "!=") {
//        error("relation operator");
//    }
//}

//void Grammar::Letter() {
//    if (str.length() != 1) {
//        error("1 length letter");
//    }
//    if (!isalpha(str[0]) && str != "_") {
//        error("letter or '_'");
//    }
//}

//void Grammar::Digit() {
//    if (str.length() != 1) {
//        error("1 length digit");
//    }
//    if (!isdigit(str[0])) {
//        error("digit");
//    }
//}

//void Grammar::Char() {
//    if (sym != "CHARCON") {
//        error("char");
//    }
//}

//void Grammar::Str() {
//    if (sym != "STRCON") {
//        error("string");
//    }
//}

void Grammar::Program() {
    if (sym == "CONSTTK") {
        ConstDeclare();
        next_sym();
    }
    if (sym == "INTTK" || sym == "CHARTK" || sym == "VOIDTK") {
        bool isVoid = (sym == "VOIDTK");
        next_sym();
        next_sym();
        next_sym();
        if (sym != "LPARENT") {
            retract();
            retract();
            retract();
            VariableDeclare();
        }
        else {
            retract();
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
                        break;
                    } else {
                        retract();
                        NonRetFuncDef();
                    }
                }
                next_sym();
                retract();
            }
        }
    }
    output("<程序>");
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
    if (sym == "PLUS" || sym == "MINUS") {
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
    while (sym == "INTTK" || sym == "CHARTK") {
        next_sym();
        VariableDef();
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

//void Grammar::VariableDefNoInit() {
//    bool first = true;
//
//    while (sym == "COMMA" || first) {
//        if (first) {
//            first = false;
//            TypeIdentifier();
//        }
//        next_sym();
//        Identifier();
//        next_sym();
//        if (sym == "LBRACK") {
//            next_sym();
//            UnsignedInt();
//            next_sym();
//            if (sym != "RBRACK") {
//                error("']'");
//            }
//            next_sym();
//            if (sym == "LBRACK") {
//                next_sym();
//                UnsignedInt();
//                next_sym();
//                if (sym != "RBRACK") {
//                    error("']'");
//                }
//            } else {
//                retract();
//            }
//        } else {
//            retract();
//        }
//    }
//
//    output("<变量定义无初始化>");
//}
//
//void Grammar::VariableDefInit() {
//    TypeIdentifier();
//    next_sym();
//    Identifier();
//    next_sym();
//    if (sym == "ASSIGN") {
//        next_sym();
//        Const();
//    }
//    else if (sym == "LBRACK") {
//        next_sym();
//        UnsignedInt();
//        next_sym();
//        if (sym != "RBRACK") {
//            error("']'");
//        }
//        next_sym();
//        if (sym == "ASSIGN") {
//            next_sym();
//            if (sym != "LBRACE") {
//                error("'{'");
//            }
//            do {
//                next_sym();
//                Const();
//                next_sym();
//            } while (sym == "COMMA");
//            //已预读
//            if (sym != "RBRACE") {
//                error("'}'");
//            }
//        }
//        else if (sym == "LBRACK") {
//            next_sym();
//            UnsignedInt();
//            next_sym();
//            if (sym != "RBRACK") {
//                error("']'");
//            }
//            next_sym();
//            if (sym != "ASSIGN") {
//                error("'='");
//            }
//            next_sym();
//            if (sym != "LBRACE") {
//                error("'{'");
//            }
//            do {
//                next_sym();
//                if (sym != "LBRACE") {
//                    error("'{'");
//                }
//                do {
//                    next_sym();
//                    Const();
//                    next_sym();
//                } while (sym == "COMMA");
//                //已预读
//                if (sym != "RBRACE") {
//                    error("'}'");
//                }
//            } while (sym == "COMMA");
//            //已预读
//            if (sym != "RBRACE") {
//                error("'}'");
//            }
//        }
//        else {
//            error("array definition with init");
//        }
//    }
//    else {
//        error("variable definition with init");
//    }
//
//    output("<变量定义及初始化>");
//}

void Grammar::TypeIdentifier() {
    if (sym != "INTTK" && sym != "CHARTK") {
        error("'int' or 'char'");
    }
}

void Grammar::RetFuncDef() {
    DeclareHead();
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
        //assert: 语句不以INTTK CHARTK开头
    }
    StmtList();
    output("<复合语句>");
    retract();
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
    if (sym == "IDENTF") {
        next_sym();
        if (sym == "LPARENT") {
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
    else if (sym == "IDENTF") {
        next_sym();
        if (sym == "LPARENT") {
            //TODO: 有返回值 or 无返回值 —— 建表
        }
        else if (sym == "ASSIGN" || sym == "LBRACK") {
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






