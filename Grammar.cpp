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
            local_addr = LOCAL_ADDR_INIT;
            while (sym == "INTTK" || sym == "CHARTK" || sym == "VOIDTK") {
                if (sym == "INTTK" || sym == "CHARTK") {
                    RetFuncDef();
                } else {
                    next_sym();
                    if (sym == "MAINTK") {
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
            Token tk2 = tk;
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            value = Int();
            SymTable::add_const(cur_func, tk2, integer, value);
            next_sym();
        } while (sym == "COMMA");
    } else if (sym == "CHARTK") {
        do {
            next_sym();
            id = Identifier();
            Token tk2 = tk;
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            next_sym();
            if (sym != "CHARCON") {
                error("char");
            }
            SymTable::add_const(cur_func, tk2, character, to_string(tk.v_char));
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

pair<DataType, string> Grammar::Const() {
    string ret;
    DataType dt = integer;
    if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        ret = Int();
    } else if (sym == "CHARCON") {
        ret = "'" + tk.str + "'";
        dt = character;
    } else {
        error("char");
    }

    output("<常量>");
    return make_pair(dt, ret);
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
        pair<DataType, string> con = Const();
        if (con.first != dataType) {
            error("const type");
        }
        init = true;
        SymTable::add(cur_func, tk2, var, dataType, local_addr);
        local_addr += size_of(dataType);
        add_midcode(OP_ASSIGN, id, con.second, VACANT);
    } else if (sym == "LBRACK") {
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
                pair<DataType, string> con = Const();
                add_midcode(OP_ARR_SAVE, id, to_string(count - 1), con.second);
                if (con.first != dataType) {
                    error("const type");
                }
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
            SymTable::add(cur_func, tk2, var, dataType, local_addr, tmp_dim1, 0);
            if (cur_func != GLOBAL) {
                local_addr += size_of(dataType) * tmp_dim1;
            }
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
                        pair<DataType, string> con = Const();
                        add_2d_array(OP_ARR_SAVE, id, to_string(dim1_count - 1),to_string(dim2_count - 1), con.second, tmp_dim2);
                        if (con.first != dataType) {
                            error("const type");
                        }
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
                SymTable::add(cur_func, tk2, var, dataType, local_addr, tmp_dim1, tmp_dim2);
                if (cur_func != GLOBAL) {
                    local_addr += size_of(dataType) * tmp_dim1 * tmp_dim2;
                }
            } else {    //int a[3][3];
                retract();
                init = false;
                SymTable::add(cur_func, tk2, var, dataType, local_addr, tmp_dim1, tmp_dim2);
                if (cur_func != GLOBAL) {
                    local_addr += size_of(dataType) * tmp_dim1 * tmp_dim2;
                }
            }
        } else {  //int a[3];
            retract();
            init = false;
            SymTable::add(cur_func, tk2, var, dataType, local_addr, tmp_dim1, 0);
            if (cur_func != GLOBAL) {
                local_addr += size_of(dataType) * tmp_dim1;
            }
        }
    } else {  //int a;
        retract();
        init = false;
        SymTable::add(cur_func, tk2, var, dataType, local_addr);
        local_addr += size_of(dataType);
    }

    if (init) {
        output("<变量定义及初始化>");
    } else {  //int a,b,c;
        next_sym();
        while (sym == "COMMA") {
            next_sym();
            Identifier();
            Token tk3 = tk;
            next_sym();
            if (sym == "LBRACK") {
                next_sym();
                UnsignedInt();
                tmp_dim1 = tk.v_int;
                next_sym();
                if (sym != "RBRACK") {
                    error("']'");
                }
                next_sym();
                if (sym == "LBRACK") {
                    next_sym();
                    UnsignedInt();
                    tmp_dim2 = tk.v_int;
                    next_sym();
                    if (sym != "RBRACK") {
                        error("']'");
                    }
                    SymTable::add(cur_func, tk3, var, dataType, local_addr, tmp_dim1, tmp_dim2);
                    if (cur_func != GLOBAL) {
                        local_addr += size_of(dataType) * tmp_dim1 * tmp_dim2;
                    }
                } else {
                    retract();
                    SymTable::add(cur_func, tk3, var, dataType, local_addr, tmp_dim1, 0);
                    if (cur_func != GLOBAL) {
                        local_addr += size_of(dataType) * tmp_dim1;
                    }
                }
            } else {
                SymTable::add(cur_func, tk3, var, dataType, local_addr);
                local_addr += size_of(dataType);
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
    local_addr = LOCAL_ADDR_INIT;
}

void Grammar::SharedFuncDefHead() {
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
    cur_func = tk.str;
    int idx = SymTable::add_func(tk, ret_type, tmp_para_types);
    add_midcode(OP_FUNC, type_to_str(ret_type), tk.str, VACANT);
    output("<声明头部>");

    next_sym();
    SharedFuncDefHead();
    if (idx != -1) {
        SymTable::global[idx].types = tmp_para_types;
    }
    next_sym();
    SharedFuncDefBody();
    if (!has_returned) {
        error("ret return");
    }

    tmp_para_types.clear();
    funcdef_ret = invalid;
    has_returned = false;

    add_midcode(OP_END_FUNC, ret_type == integer ? "int" : "char", cur_func, VACANT);

    output("<有返回值函数定义>");
}

void Grammar::NonRetFuncDef() {
    funcdef_ret = void_ret;
    if (sym != "VOIDTK") {
        error("'void");
    }

    next_sym();
    Identifier();
    cur_func = tk.str;
    int idx = SymTable::add_func(tk, void_ret, tmp_para_types);
    add_midcode(OP_FUNC, "void", tk.str, VACANT);

    next_sym();
    SharedFuncDefHead();
    if (idx != -1) {
        SymTable::global[idx].types = tmp_para_types;
    }
    next_sym();
    SharedFuncDefBody();

    tmp_para_types.clear();
    funcdef_ret = invalid;
    has_returned = false;

    add_midcode(OP_RETURN, VACANT, VACANT, VACANT); //无返回值函数结尾补充return
    add_midcode(OP_END_FUNC, "void", cur_func, VACANT);

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
    SymTable::add(cur_func, tk, para, dataType, local_addr);
//    add_midcode(OP_PARA, type_to_str(dataType), tk.str, VACANT);
    local_addr += size_of(dataType);
    tmp_para_count = 1;
    next_sym();
    while (sym == "COMMA") {
        next_sym();
        TypeIdentifier();
        DataType dataType2 = (sym == "INTTK") ? integer : character;
        tmp_para_types.push_back(dataType2);
        next_sym();
        Identifier();
        SymTable::add(cur_func, tk, para, dataType2, local_addr);
//        add_midcode(OP_PARA, type_to_str(dataType), tk.str, VACANT);
        local_addr += size_of(dataType2);
        next_sym();
        tmp_para_count++;
    }

    output("<参数表>");
    retract();
    //空
}

void Grammar::Main() {
    SymTable::add_func(tk, void_ret, tmp_para_types);
    add_midcode(OP_FUNC, "void", "main", VACANT);
    funcdef_ret = void_ret;
    cur_func = "main";
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
    add_midcode(OP_RETURN, VACANT, VACANT, VACANT); //无返回值函数结尾补充return
    add_midcode(OP_END_FUNC, "void", "main", VACANT);

    output("<主函数>");
    local_addr = LOCAL_ADDR_INIT;
    funcdef_ret = invalid;
}

pair<DataType, string> Grammar::Expr() {
    DataType ret_type = invalid;
    bool neg = false;
    if (sym == "PLUS" || sym == "MINU") {
        ret_type = integer;
        neg = sym == "MINU";
        next_sym();
    }
    pair<DataType, string> item = Item();
    string num1 = item.second;
    if (item.first == character && ret_type == invalid) {
        ret_type = character;
    }
    if (neg) {
        num1 = add_midcode(OP_SUB, "0", num1, AUTO);
        SymTable::add(cur_func, num1, tmp, integer, local_addr);
        local_addr += 4;
    }
    next_sym();
    while (sym == "PLUS" || sym == "MINU") {
        string op = sym == "PLUS" ? OP_ADD : OP_SUB;
        ret_type = integer;
        next_sym();
        string num2 = Item().second;
        if (op == OP_SUB) {
            num1 = add_sub(num1, num2);
        } else {
            num1 = add_midcode(op, num1, num2, AUTO);
            SymTable::add(cur_func, num1, tmp, integer, local_addr);
            local_addr += 4;
        }
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

        if (op == OP_DIV && num_or_char(const_replace(num1)) && !num_or_char(const_replace(num2))) {
            //y=5/x:  z=0+5; y=z/x
            num1 = add_midcode(OP_ADD, "0", num1, AUTO);
            SymTable::add(cur_func, num1, tmp, integer, local_addr);
            local_addr += 4;
            num1 = add_midcode(OP_DIV, num1, num2, AUTO);
        } else {
            num1 = add_midcode(op, num1, num2, AUTO);
        }
        SymTable::add(cur_func, num1, tmp, integer, local_addr);
        local_addr += 4;
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
        SymTableItem item = SymTable::search(cur_func, tk);
        if (item.dataType == character) {
            ret_type = character;
        } else {
            ret_type = integer;
        }
        ret_str = tk.str;
        next_sym(); // func(
        if (sym == "LPARENT") {
            retract(); //func
            RetFuncCall();
            ret_str = add_midcode(OP_ADD, "%RET", "0", AUTO);
            SymTable::add(cur_func, ret_str, tmp, ret_type, local_addr);
            local_addr += 4;
        } else if (sym == "LBRACK") {
            next_sym();
            pair<DataType, string> index1 = Expr();
            if (index1.first != integer) {
                error("array index type");
            }
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            next_sym();
            if (sym == "LBRACK") {
                //a[2][2]
                next_sym();
                pair<DataType, string> index2 = Expr();
                if (index2.first != integer) {
                    error("array index type");
                }
                next_sym();
                if (sym != "RBRACK") {
                    error("']'");
                }
                ret_str = add_2d_array(OP_ARR_LOAD, ret_str, index1.second, index2.second, AUTO);
                SymTable::add(cur_func, ret_str, tmp, ret_type, local_addr);
                local_addr += 4;
            } else {
                //a[2]
                retract();
                ret_str = add_midcode(OP_ARR_LOAD, ret_str, index1.second, AUTO);
                SymTable::add(cur_func, ret_str, tmp, ret_type, local_addr);
                local_addr += 4;
            }
        } else {
            retract();
        }
    } else if (sym == "LPARENT") {
        ret_type = integer;
        //ret_str = "(";
        next_sym();
        ret_str += Expr().second;
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        //ret_str += ")";
    } else if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        ret_type = integer;
        ret_str = Int();
    } else if (sym == "CHARCON") {
        ret_type = character;
        ret_str = "'" + tk.str + "'";
    } else {
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
            if (SymTable::search(GLOBAL, tk).stiType != func) {
                //error("function call");
                while (sym != "SEMICN") {
                    next_sym();
                }
                retract();
            } else if (SymTable::search(GLOBAL, tk).dataType != void_ret) {
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
    if (SymTable::search(cur_func, tk).stiType == constant) {
        error("change const");
    }
    next_sym();
    if (sym == "ASSIGN") {
        next_sym();
        value = Expr().second;
        add_midcode(OP_ASSIGN, id, value, VACANT);
    } else if (sym == "LBRACK") {
        next_sym();
        pair<DataType, string> index1 = Expr();
        if (index1.first != integer) {
            error("array index type");
        }
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        next_sym();
        if (sym == "ASSIGN") {
            //a[2]=3
            next_sym();
            value = Expr().second;
            add_midcode(OP_ARR_SAVE, id, index1.second, value);
        } else if (sym == "LBRACK") {
            //a[2][2]=3
            next_sym();
            pair<DataType, string> index2 = Expr();
            if (index2.first != integer) {
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
            value = Expr().second;
            add_2d_array(OP_ARR_SAVE, id, index1.second, index2.second, value);
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
    pair<string, string> cond = Condition();
    string label_else = add_midcode(OP_JUMP_IF, cond.first, cond.second, AUTO_LABEL);
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    next_sym();
    Stmt();
    next_sym();
    if (sym == "ELSETK") {
        string label_end = MidCodeList::assign_label();
        add_midcode(OP_JUMP_UNCOND, label_end, VACANT, VACANT);
        add_midcode(OP_LABEL, label_else, VACANT, VACANT);
        next_sym();
        Stmt();
        next_sym();
        add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    } else {
        add_midcode(OP_LABEL, label_else, VACANT, VACANT);
    }

    output("<条件语句>");
    retract();
}

pair<string, string> Grammar::Condition() {
    pair<DataType, string> it1 = Expr();
    if (it1.first != integer) {
        error("condition type");
    }
    next_sym();
    //返回的第二个值是条件运算符的取反
    string op;
    if (sym == "LSS") {
        op = ">=0";
    } else if (sym == "LEQ"){
        op = ">0";
    } else if (sym == "GRE"){
        op = "<=0";
    } else if (sym == "GEQ"){
        op = "<0";
    } else if (sym == "EQL"){
        op = "!=0";
    } else if (sym == "NEQ"){
        op = "==0";
    } else {
        error("relation operator");
    }
    next_sym();
    pair<DataType, string> it2 = Expr();
    if (it2.first != integer) {
        error("condition type");
    }

    string ret = add_sub(it1.second, it2.second);

    output("<条件>");
    return make_pair(ret, op);
}

void Grammar::LoopStmt() {

    if (sym == "WHILETK") {
        next_sym();
        if (sym != "LPARENT") {
            error("'('");
        }
        next_sym();
        string label_begin = MidCodeList::assign_label();
        add_midcode(OP_LABEL, label_begin, VACANT, VACANT);
        pair<string, string> cond = Condition();
        string label_end = add_midcode(OP_JUMP_IF, cond.first, cond.second, AUTO_LABEL);
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        next_sym();
        Stmt();
        add_midcode(OP_JUMP_UNCOND, label_begin, VACANT, VACANT);
        add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    }

    else if (sym == "FORTK") {
        next_sym();
        if (sym != "LPARENT") {
            error("'('");
        }
        next_sym();
        string id = Identifier();
        SymTable::search(cur_func, tk);

        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        next_sym();
        string value = Expr().second;
        add_midcode(OP_ASSIGN, id, value, VACANT);
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        next_sym();
        string label_begin = MidCodeList::assign_label();
        add_midcode(OP_LABEL, label_begin, VACANT, VACANT);
        pair<string, string> cond = Condition();
        string label_end = add_midcode(OP_JUMP_IF, cond.first, cond.second, AUTO_LABEL);
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }

        next_sym();
        id = Identifier();
        SymTable::search(cur_func, tk);
        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        next_sym();
        string id2 = Identifier();
        SymTable::search(cur_func, tk);
        next_sym();
        string op = sym == "PLUS" ? OP_ADD : OP_SUB;
        if (sym != "PLUS" && sym != "MINU") {
            error("'+' or '-'");
        }
        next_sym();
        UnsignedInt();
        string pace_length = tk.str;
        output("<步长>");

        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        next_sym();
        Stmt();
        add_midcode(op, id2, pace_length, id);
        add_midcode(OP_JUMP_UNCOND, label_begin, VACANT, VACANT);
        add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    } else {
        error("'while' or 'for'");
    }

    output("<循环语句>");
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
    pair<DataType, string> expr = Expr();
    SymTableItem it = SymTable::ref_search(cur_func, expr.second);
    if (it.stiType == tmp) {
        it.stiType = var; //switch的表达式不能被放在t寄存器
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
    string label_end = MidCodeList::assign_label();


    if (sym != "CASETK") {
        error("'case'");
    }
    next_sym();
    pair<DataType, string> con = Const();
    if (con.first != expr.first) {
        error("const type");
    }
    string r = add_sub(expr.second, con.second);
    string label_next = add_midcode(OP_JUMP_IF, r, "!=0", AUTO_LABEL);
    next_sym();
    if (sym != "COLON") {
        error("':'");
    }
    next_sym();
    Stmt();
    add_midcode(OP_JUMP_UNCOND, label_end, VACANT, VACANT);
    output("<情况子语句>");
    next_sym();
    while (sym == "CASETK") {
        if (sym != "CASETK") {
            error("'case'");
        }
        next_sym();
        con = Const();
        if (con.first != expr.first) {
            error("const type");
        }
        add_midcode(OP_LABEL, label_next, VACANT, VACANT);
        r = add_sub(expr.second, con.second);
        label_next = add_midcode(OP_JUMP_IF, r, "!=0", AUTO_LABEL);
        next_sym();
        if (sym != "COLON") {
            error("':'");
        }
        next_sym();
        Stmt();
        add_midcode(OP_JUMP_UNCOND, label_end, VACANT, VACANT);
        output("<情况子语句>");
        next_sym();
    }
    retract();
    output("<情况表>");

    next_sym();
    if (sym != "DEFAULTTK") {
        error("'default'");
        retract();
    }
    else {
        next_sym();
        if (sym != "COLON") {
            error("':'");
        }
        next_sym();
        add_midcode(OP_LABEL, label_next, VACANT, VACANT);
        Stmt();
        output("<缺省>");
    }

    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    output("<情况语句>");
}

void Grammar::SharedFuncCall() {
    bool para_count_err = false;
    string func_name = Identifier();
    add_midcode(OP_PREPARE_CALL, func_name, VACANT, VACANT);
    vector<DataType> types = SymTable::search(cur_func, tk).types;
    pair<DataType, string> expr;
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
            expr = Expr();
            add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
        } else {
            expr = Expr();
            if (*(types.begin()) != expr.first) {
                error("para type");
            }
            types.erase(types.begin());
            add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
        }
        next_sym();
        while (sym == "COMMA") {
            next_sym();
            if (types.begin() == types.end()) {
                para_count_err = true;
                expr = Expr();
                add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
            } else {
                expr = Expr();
                if (*(types.begin()) != expr.first) {
                    error("para type");
                }
                types.erase(types.begin());
                add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
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

    add_midcode(OP_CALL, func_name, VACANT, VACANT);
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
    add_midcode(OP_SCANF, tk.str, VACANT, VACANT);
    if (SymTable::search(cur_func, tk).stiType == constant) {
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
        MidCodeList::strcons.push_back(str_replace(tk.str, "\\", "\\\\"));
        add_midcode(OP_PRINT, to_string(MidCodeList::strcons.size() - 1), "strcon", VACANT);
        output("<字符串>");
        next_sym();
        if (sym == "COMMA") {
            next_sym();
            pair<DataType, string> p = Expr();
            add_midcode(OP_PRINT, p.second, p.first == character ? "char" : "int", VACANT);
            next_sym();
            if (sym != "RPARENT") {
                error("')'");
            }
        } else if (sym != "RPARENT") {
            error("')'");
        }
    } else {
        pair<DataType, string> p = Expr();
        add_midcode(OP_PRINT, p.second, p.first == character ? "char" : "int", VACANT);
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
    }
    add_midcode(OP_PRINT, ENDL, VACANT, VACANT);

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
        pair<DataType, string> expr = Expr();
        if (expr.first != funcdef_ret && funcdef_ret != void_ret) {
            error("ret return");
        }
        add_midcode(OP_RETURN, expr.second, VACANT, VACANT);
        MidCodeList::ret_value = expr.second;
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        next_sym();
    } else {
        if (funcdef_ret != void_ret) {
            error("ret return");
        }
        add_midcode(OP_RETURN, VACANT, VACANT, VACANT);
    }
    has_returned = true;

    output("<返回语句>");
    retract();
}

string Grammar::add_midcode(const string &op, const string &n1, const string &n2, const string &r) const {
    string num1 = const_replace(n1);
    string num2 = const_replace(n2);
    string result = r;
    if (op == OP_ARR_SAVE) {
        result = const_replace(r);
    }
    return MidCodeList::add(op, num1, num2, result);
}

string Grammar::const_replace(string symbol) const {
    if (symbol == VACANT) {
        return symbol;
    }
    SymTableItem item = SymTable::try_search(cur_func, symbol, true);
    if (item.valid && item.stiType == constant) {
        return item.const_value;
    }
    return symbol;
}

string Grammar::add_sub(const string& num1, const string& num2) {
    string ret;
    if (num_or_char(const_replace(num1)) && !num_or_char(const_replace(num2))) {
        //y=5-x:  z=x-5; y=0-z
        ret = add_midcode(OP_SUB, num2, num1, AUTO);
        SymTable::add(cur_func, ret, tmp, integer, local_addr);
        local_addr += 4;
        ret = add_midcode(OP_SUB, "0", ret, AUTO);
    } else {
        ret = add_midcode(OP_SUB, num1, num2, AUTO);
    }
    SymTable::add(cur_func, ret, tmp, integer, local_addr);
    local_addr += 4;
    return ret;
}

string Grammar::add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r, int dim2_size) {
    //二维转一维
    assertion(op == OP_ARR_LOAD || op == OP_ARR_SAVE);
    assertion(n3 != VACANT);
    string num1 = const_replace(n1);
    string num2 = const_replace(n2);
    string num3 = const_replace(n3);

    string mid = add_midcode(OP_MUL, num2, to_string(dim2_size), AUTO);
    SymTable::add(cur_func, mid, tmp, integer, local_addr);
    local_addr += 4;
    mid = add_midcode(OP_ADD, mid, num3, AUTO);
    SymTable::add(cur_func, mid, tmp, integer, local_addr);
    local_addr += 4;
    return add_midcode(op, n1, mid, r);
}

string Grammar::add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r) {
    return add_2d_array(op, n1, n2, n3, r, SymTable::search(cur_func, n1).dim2_size);
}

