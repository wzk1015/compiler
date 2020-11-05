#include "Grammar.h"

int Grammar::analyze() {
    ofstream out;
    if (out_path != INVALID) {
        out.open(out_path);
    }

    try {
        next_sym();
        Program();
    } catch (exception) {
        Errors::add("unexpected end of tokens", E_UNEXPECTED_EOF);
    }

    while (lexer.pos < lexer.source.length() - 1) {
        char a = lexer.source[lexer.pos];
        if (isspace(a)) {
            lexer.read_char();
            continue;
        }
        Errors::add("unexpected character '" + string(&a) + "' at end of file",
                    lexer.line_num, lexer.col_num, E_UNEXPECTED_CHAR);
        break;
    }

    if (out_path != INVALID) {
        for (auto &str: output_str) {
            out << str << endl;
        }
    }
    out.close();

    cout << "Grammar complete successfully." << endl;
    return 0;
}

void Grammar::output(const string &str) {
    output_str.push_back(str);
//    if (DEBUG) {
//        cout << str << endl;
//    }
}

int Grammar::next_sym() {
    if (pos < cur_lex_results.size()) {
        tk = cur_lex_results[pos];
    } else {
        tk = lexer.analyze();
        if (tk.type == INVALID) {
            throw exception();
        }
        cur_lex_results.push_back(tk);
    }
    pos++;
    sym = tk.type;
    output_str.push_back(tk.type + " " + tk.str);

//    if (DEBUG) {
//        cout << tk.type + " " + tk.str << endl;
//    }
    return 0;
}

void Grammar::retract() {
    pos--;
    tk = cur_lex_results[pos - 1];
    sym = tk.type;
    for (unsigned int i = output_str.size() - 1; i >= 0; i--) {
        if (output_str[i][0] != '<') {  //词法分析输出
//            if (DEBUG) {
//                cout << "retract " << output_str[i] << endl;
//            }
            output_str.erase(output_str.begin() + i);
            break;
        }
    }
}

void Grammar::error(const string &expected) {
    if (expected == "'default'") {
        Errors::add("Expected " + expected + ", but got '" + tk.str + "' (type: " + sym + ")",
                    tk.line, tk.column, ERR_SWITCH_DEFAULT);
    } else if (expected == "')'") {
        Errors::add("Expected " + expected + ", but got '" + tk.str + "' (type: " + sym + ")",
                    tk.line, tk.column, ERR_RPARENT);
        retract();
    } else if (expected == "']'") {
        Errors::add("Expected " + expected + ", but got '" + tk.str + "' (type: " + sym + ")",
                    tk.line, tk.column, ERR_RBRACK);
        retract();
    } else if (expected == "';'") {
        retract();
        Errors::add("Expected " + expected + ", but got '" + tk.str + "' (type: " + sym + ")",
                    tk.line, tk.column, ERR_SEMICOL);
    } else if (expected == "array init") {
        Errors::add("Array init mismatch", tk.line, tk.column, ERR_ARRAY_INIT);
    } else if (expected == "const type") {
        Errors::add("Const type mismatch", tk.line, tk.column, ERR_CONST_TYPE);
    } else if (expected == "change const") {
        Errors::add("change const", tk.line, tk.column, ERR_CONST_ASSIGN);
    } else if (expected == "para count") {
        Errors::add("Para count mismatch", tk.line, tk.column, ERR_PARA_COUNT);
    } else if (expected == "para type") {
        Errors::add("Para type mismatch", tk.line, tk.column, ERR_PARA_TYPE);
    } else if (expected == "nonret return") {
        Errors::add("void function return mismatch", tk.line, tk.column, ERR_NONRET_FUNC);
    } else if (expected == "ret return") {
        Errors::add("non-void function return mismatch", tk.line, tk.column, ERR_RET_FUNC);
    } else if (expected == "condition type") {
        Errors::add("condition type error", tk.line, tk.column, ERR_CONDITION_TYPE);
    } else if (expected == "array index type") {
        Errors::add("array index type error", tk.line, tk.column, ERR_INDEX_CHAR);
    } else {
        Errors::add("Expected " + expected + ", but got '" + tk.str + "' (type: " + sym + ")",
                    tk.line, tk.column, E_GRAMMAR);
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
    string id;
    string value;
    if (sym == "INTTK") {
        do {
            next_sym();
            id = Identifier();
            SymTable::add(tk, constant, integer);
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            value = Int();
            next_sym();
        } while (sym == "COMMA");
    } else if (sym == "CHARTK") {
        do {
            next_sym();
            id = Identifier();
            SymTable::add(tk, constant, character);
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            if (sym != "CHARCON") {
                error("char");
            }
            value = tk.str;
            next_sym();
        } while (sym == "COMMA");
    } else {
        error("const def");
    }

    output("<常量定义>");
    retract();
    MidCodeList::add(OP_ASSIGN, id, value, VACANT);
}

void Grammar::UnsignedInt() {
    if (sym != "INTCON") {
        error("unsigned int");
    }

    output("<无符号整数>");
}

string Grammar::Int() {
    string ret;
    if (sym == "PLUS" || sym == "MINU") {
        ret += tk.str;
        next_sym();
    }
    UnsignedInt();
    ret += tk.str;

    output("<整数>");
    return ret;
}

string Grammar::Identifier() {
    if (sym != "IDENFR") {
        error("identifier");
    }
    return tk.str;
}

string Grammar::Const() {
    string ret;
    if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        ret = Int();
        tmp_const_data_type = integer;
    } else if (sym == "CHARCON") {
        ret = tk.str;
        tmp_const_data_type = character;
    } else {
        error("char");
    }

    output("<常量>");
    return ret;
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
    string id;
    string value;

    TypeIdentifier();

    DataType dataType = (sym == "INTTK") ? integer : character;

    next_sym();
    id = Identifier();
    Token tk2 = tk;

    next_sym();
    if (sym == "ASSIGN") {  //int a=1;
        next_sym();
        value = Const();
        if (tmp_const_data_type != dataType) {
            error("const type");
        }
        tmp_const_data_type = invalid;
        init = true;
        SymTable::add(tk2, var, dataType);
        MidCodeList::add(OP_ASSIGN, id, value, VACANT);
    }
    else if (sym == "LBRACK") {
        //TODO
        next_sym();
        UnsignedInt();
        tmp_dim1 = tk.v_int;
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        next_sym();
        if (sym == "ASSIGN") {  //int a[3]={1,2,3}
            next_sym();
            if (sym != "LBRACE") {
//                error("'{'");
                error("array init");
            }
            int count = 0;
            do {
                count++;
                if (count > tmp_dim1) {
                    error("array init");
                }
                next_sym();
                Const();
                if (tmp_const_data_type != dataType) {
                    error("const type");
                }
                tmp_const_data_type = invalid;
                next_sym();
            } while (sym == "COMMA");
            if (count < tmp_dim1) {
                error("array init");
            }
            //已预读
            if (sym != "RBRACE") {
                error("'}'");
            }
            init = true;
            SymTable::add(tk2, var, dataType, 1);
        } else if (sym == "LBRACK") {
            next_sym();
            UnsignedInt();
            tmp_dim2 = tk.v_int;
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
            if (sym == "ASSIGN") { //int a[3][3]={{1,2,3}, {4,5,6}, {7,8,9}}
                next_sym();
                if (sym != "LBRACE") {
//                    error("'{'");
                    error("array init");
                }
                int dim1_count = 0;
                int dim2_count = 0;
                do {
                    dim1_count++;
                    if (dim1_count > tmp_dim1) {
                        error("array init");
                    }
                    dim2_count = 0;
                    next_sym();
                    if (sym != "LBRACE") {
//                        error("'{'");
                        error("array init");
                    }
                    do {
                        dim2_count++;
                        if (dim2_count > tmp_dim2) {
                            error("array init");
                        }
                        next_sym();
                        Const();
                        if (tmp_const_data_type != dataType) {
                            error("const type");
                        }
                        tmp_const_data_type = invalid;
                        next_sym();
                    } while (sym == "COMMA");
                    //已预读
                    if (dim2_count < tmp_dim2) {
                        error("array init");
                    }
                    if (sym != "RBRACE") {
                        error("'}'");
                    }
                    next_sym();
                } while (sym == "COMMA");
                //已预读
                if (dim1_count < tmp_dim1) {
                    error("array init");
                }
                if (sym != "RBRACE") {
                    error("'}'");
                }
                init = true;
                SymTable::add(tk2, var, dataType, 2);
            } else {    //int a[3][3];
                retract();
                init = false;
                SymTable::add(tk2, var, dataType, 2);
            }
        } else {  //int a[3];
            retract();
            init = false;
            SymTable::add(tk2, var, dataType, 1);
        }
    }
    else {  //int a;
        retract();
        init = false;
        SymTable::add(tk2, var, dataType);
    }

    if (init) {
        output("<变量定义及初始化>");
    }
    else {  //int a,b,c;
        next_sym();
        while (sym == "COMMA") {
            next_sym();
            Identifier();
            Token tk3 = tk;
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
                    SymTable::add(tk3, var, dataType, 2);
                } else {
                    retract();
                    SymTable::add(tk3, var, dataType, 1);
                }
            } else {
                SymTable::add(tk3, var, dataType);
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

void Grammar::SharedFuncDefBody() {
    if (sym != "LBRACE") {
        error("'{'");
    }
    next_sym();
    CompoundStmt();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    SymTable::pop_layer();
}

void Grammar::SharedFuncDefHead() {
    SymTable::add_layer();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();

    if (sym == "INTTK" || sym == "CHARTK") {
        ParaList();
    } else if (sym == "RPARENT") {
        retract();
        output("<参数表>");
    } else {
        error("')'");
    }
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }

}

void Grammar::RetFuncDef() {
    DataType ret_type;
    if (sym == "INTTK") {
        ret_type = integer;
    } else if (sym == "CHARTK") {
        ret_type = character;
    } else {
        error("'int' or 'char'");
        ret_type = integer;
    }
    funcdef_ret = ret_type;
    next_sym();
    Identifier();
    int idx = SymTable::add_func(tk, ret_type, tmp_para_count, tmp_para_types);
    output("<声明头部>");
    next_sym();
    SharedFuncDefHead();
    SymTable::items[idx].types = tmp_para_types;
    SymTable::items[idx].num_para = tmp_para_count;
    next_sym();
    SharedFuncDefBody();
    if (!has_returned) {
        error("ret return");
    }

    tmp_para_types.clear();
    funcdef_ret = invalid;
    has_returned = false;

    output("<有返回值函数定义>");
}

void Grammar::NonRetFuncDef() {
    funcdef_ret = void_ret;
    if (sym != "VOIDTK") {
        error("'void");
    }
    next_sym();
    Identifier();
    int idx = SymTable::add_func(tk, void_ret, tmp_para_count, tmp_para_types);
    next_sym();
    SharedFuncDefHead();
    SymTable::items[idx].types = tmp_para_types;
    SymTable::items[idx].num_para = tmp_para_count;
    next_sym();
    SharedFuncDefBody();

    tmp_para_types.clear();
    funcdef_ret = invalid;
    has_returned = false;

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
    DataType dataType = (sym == "INTTK") ? integer : character;
    tmp_para_types.push_back(dataType);
    next_sym();
    Identifier();
    SymTable::add(tk, para, dataType);
    tmp_para_count = 1;
    next_sym();
    while (sym == "COMMA") {
        next_sym();
        TypeIdentifier();
        DataType dataType2 = (sym == "INTTK") ? integer : character;
        tmp_para_types.push_back(dataType2);
        next_sym();
        Identifier();
        SymTable::add(tk, para, dataType2);
        next_sym();
        tmp_para_count++;
    }

    output("<参数表>");
    retract();
    //空
}

void Grammar::Main() {
    funcdef_ret = void_ret;
    SymTable::add_layer();
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
    SymTable::pop_layer();
    funcdef_ret = invalid;
}

pair<DataType, string> Grammar::Expr() {
    DataType ret_type = invalid;
    if (sym == "PLUS" || sym == "MINU") {
        ret_type = integer;
        next_sym();
    }
    pair<DataType, string> item = Item();
    string num1 = item.second;
    if (item.first == character && ret_type == invalid) {
        ret_type = character;
    }
    next_sym();
    while (sym == "PLUS" || sym == "MINU") {
        string op = sym == "PLUS" ? OP_ADD : OP_SUB;
        ret_type = integer;
        next_sym();
        string num2 = Item().second;
        num1 = MidCodeList::add(op, num1, num2, AUTO);
        next_sym();
    }

    output("<表达式>");
    retract();
    if (ret_type == invalid) {
        ret_type = integer;
    }
    return make_pair(ret_type, num1);
}

pair<DataType, string> Grammar::Item() {
    pair<DataType, string> factor = Factor();
    DataType ret_type = factor.first;
    string num1 = factor.second;
    next_sym();
    while (sym == "MULT" || sym == "DIV") {
        string op = sym == "MULT" ? OP_MUL : OP_DIV;
        ret_type = integer;
        next_sym();
        string num2 = Factor().second;
        num1 = MidCodeList::add(op, num1, num2, AUTO);
        next_sym();
    }

    output("<项>");
    retract();
    return make_pair(ret_type, num1);
}

pair<DataType, string> Grammar::Factor() {
    DataType ret_type = integer;
    string ret_str;
    if (sym == "IDENFR") {
        if (SymTable::search(tk).dataType == character) {
            ret_type = character;
        } else {
            ret_type = integer;
        }
        ret_str = tk.str;
        next_sym(); // func(
        if (sym == "LPARENT") {
            //TODO
            retract(); //func
            RetFuncCall();
        }
        else if (sym == "LBRACK") {
            next_sym();
            //TODO
            if (Expr().first != integer) {
                error("array index type");
            }
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
            if (sym == "LBRACK") {
                //TODO
                next_sym();
                if (Expr().first != integer) {
                    error("array index type");
                }
                next_sym();
                if (sym != "RBRACK") {
                    error("']'");
                }
            } else {
                retract();
            }
        }
        else {
            retract();
        }
    }

    else if (sym == "LPARENT") {
        ret_type = integer;
        //ret_str = "(";
        next_sym();
        ret_str += Expr().second;
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        //ret_str += ")";
    }

    else if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        ret_type = integer;
        ret_str = Int();
    }

    else if (sym == "CHARCON") {
        ret_type = character;
        ret_str = tk.str;
    }

    else {
        error("factor");
    }

    output("<因子>");
    return make_pair(ret_type, ret_str);
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
            if (SymTable::search(tk).stiType != func) {
                //error("function call");
                while (sym != "SEMICN") {
                    next_sym();
                }
                retract();
            } else if (SymTable::search(tk).dataType != void_ret) {
                RetFuncCall();
            } else {
                NonRetFuncCall();   //including undefined
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
    string id = Identifier();
    string value;
    if (SymTable::search(tk).stiType == constant) {
        error("change const");
    }
    next_sym();
    if (sym == "ASSIGN") {
        next_sym();
        value = Expr().second;
        MidCodeList::add(OP_ASSIGN, id, value, VACANT);
    }
    else if (sym == "LBRACK") {
        //TODO
        next_sym();
        if (Expr().first != integer) {
            error("array index type");
        }
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
            if (Expr().first != integer) {
                error("array index type");
            }
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
    }
    else {
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
    if (Expr().first != integer) {
        error("condition type");
    }
    next_sym();
    if (sym != "LSS" && sym != "LEQ" && sym != "GRE" && sym != "GEQ" && sym != "EQL" && sym != "NEQ") {
        error("relation operator");
    }
    next_sym();
    if (Expr().first != integer) {
        error("condition type");
    }

    output("<条件>");
}

void Grammar::LoopStmt() {

    if (sym == "WHILETK") {
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
    }
    else if (sym == "FORTK") {
        next_sym();
        if (sym != "LPARENT") {
            error("'('");
        }
        next_sym();
        Identifier();
        SymTable::search(tk);
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
        SymTable::search(tk);
        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        next_sym();
        Identifier();
        SymTable::search(tk);
        next_sym();
        if (sym != "PLUS" && sym != "MINU") {
            error("'+' or '-'");
        }
        next_sym();
        PaceLength();
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
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
    tmp_switch_data_type = Expr().first;
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
    tmp_switch_data_type = invalid;
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
    if (tmp_const_data_type != tmp_switch_data_type) {
        error("const type");
        tmp_switch_data_type = invalid;
        tmp_const_data_type = invalid;
    }
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
        retract();
        return;
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
    bool para_count_err = false;
    Identifier();
    vector<DataType> types = SymTable::search(tk).types;
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    next_sym();
    if (sym == "RPARENT") {
        retract();
        output("<值参数表>");
    } else {
        if (types.begin() == types.end()) {
//            error("para count");
            para_count_err = true;
            Expr();
        } else {
            if (*(types.begin()) != Expr().first) {
                error("para type");
            }
            types.erase(types.begin());
        }
        next_sym();
        while (sym == "COMMA") {
            next_sym();
            if (types.begin() == types.end()) {
                para_count_err = true;
                Expr();
            } else {
                if (*(types.begin()) != Expr().first) {
                    error("para type");
                }
                types.erase(types.begin());
            }
            next_sym();
        }
        output("<值参数表>");
        retract();
        //空
    }
    next_sym();
    if (!types.empty()) {
        para_count_err = true;
    }
    if (sym != "RPARENT") {
        error("')'");
    } else if (para_count_err) {
        error("para count");
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
    MidCodeList::add(OP_SCANF, tk.str, VACANT, VACANT);
    if (SymTable::search(tk).stiType == constant) {
        error("change const");
    }
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
        MidCodeList::add(OP_PRINT, "'" + tk.str + "'", VACANT, VACANT);
        output("<字符串>");
        next_sym();
        if (sym == "COMMA") {
            next_sym();
            Expr();
            MidCodeList::add(OP_PRINT, tk.str, VACANT, VACANT); // TODO :TK.STR -> EXPR
            next_sym();
            if (sym != "RPARENT") {
                error("')'");
            }
        } else if (sym != "RPARENT") {
            error("')'");
        }
    } else {
        Expr();
        MidCodeList::add(OP_PRINT, tk.str, VACANT, VACANT);  // TODO :TK.STR -> EXPR
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
        if (funcdef_ret == void_ret) {
            error("nonret return");
        }
        next_sym();
        if (sym == "RPARENT" && funcdef_ret != void_ret) {
            error("ret return");
        }
        if (Expr().first != funcdef_ret && funcdef_ret != void_ret) {
            error("ret return");
        }
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        next_sym();
    } else {
        if (funcdef_ret != void_ret) {
            error("ret return");
        }
    }
    has_returned = true;

    output("<返回语句>");
    retract();
}





