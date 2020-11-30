#include "Grammar.h"

vector<FunctionIndex> Grammar::function_tokens_index;

int Grammar::next_sym(bool add_to_new = true) {
    if (pos < cur_lex_results.size()) {
        tk = cur_lex_results[pos];
    } else {
        tk = lexer.analyze();
        if (tk.type == INVALID) {
            throw exception();
        }
        cur_lex_results.push_back(tk);
        if (add_to_new) {
            new_lex_results.push_back(tk);
        }
    }
    pos++;
    sym = tk.type;
    output_str.push_back(tk.type + " " + tk.original_str);

//    if (DEBUG) {
//        cout << tk.type + " " + tk.str << endl;
//    }
    return 0;
}

int Grammar::analyze() {
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

    cout << "Grammar complete successfully." << endl;
    return 0;
}

void Grammar::save_to_file(const string &out_path) {
    ofstream out(out_path);
    for (auto &str: output_str) {
        out << str << endl;
    }
    out.close();
}

void Grammar::output(const string &str) {
    output_str.push_back(str);
//    if (DEBUG) {
//        cout << str << endl;
//    }
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
    cout << "mode: " << mode << endl;
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
    nodes.emplace_back("<程序>", INVALID, -1);
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
    add_node("<常量说明>");
    do {
        if (sym != "CONSTTK") {
            error("'const'");
        }
        add_leaf();
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
    tree_backward();
}

void Grammar::ConstDef() {
    add_node("<常量定义>");
    string id;
    string value;
    if (sym == "INTTK") {
        add_leaf();
        do {
            next_sym();
            id = Identifier();
            Token tk2 = tk;
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            add_leaf();
            next_sym();
            value = Int();
            SymTable::add_const(cur_func, tk2, integer, value);
            next_sym();
        } while (sym == "COMMA");
    } else if (sym == "CHARTK") {
        add_leaf();
        do {
            next_sym();
            id = Identifier();
            Token tk2 = tk;
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            add_leaf();
            next_sym();
            if (sym != "CHARCON") {
                error("char");
            }
            add_leaf();
            SymTable::add_const(cur_func, tk2, character, to_string(tk.v_char));
            next_sym();
        } while (sym == "COMMA");
    } else {
        error("const def");
    }

    output("<常量定义>");
    retract();
    tree_backward();
}

void Grammar::UnsignedInt() {
    add_node("<无符号整数>");
    if (sym != "INTCON") {
        error("unsigned int");
    }

    output("<无符号整数>");
    tree_backward();
}

string Grammar::Int() {
    add_node("<整数>");
    string ret;
    if (sym == "PLUS" || sym == "MINU") {
        ret += tk.str;
        next_sym();
    }
    UnsignedInt();
    ret += tk.str;

    output("<整数>");
    tree_backward();
    return ret;
}

string Grammar::Identifier() {
    add_node("<标识符>");
    if (sym != "IDENFR") {
        error("identifier");
    }
    tree_backward();
    return tk.str;
}

pair<DataType, string> Grammar::Const() {
    add_node("<常量>");
    string ret;
    DataType dt = integer;
    if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        ret = Int();
    } else if (sym == "CHARCON") {
        add_leaf();
        ret = "'" + tk.str + "'";
        dt = character;
    } else {
        error("char");
    }

    output("<常量>");
    tree_backward();
    return make_pair(dt, ret);
}

void Grammar::VariableDeclare() {
    add_node("<变量说明>");
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
    tree_backward();
}

void Grammar::VariableDef() {
    add_node("<变量定义>");
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
        add_leaf();
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
        add_leaf();
        next_sym();
        UnsignedInt();
        tmp_dim1 = tk.v_int;
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        add_leaf();
        next_sym();
        if (sym == "ASSIGN") {  //int a[3]={1,2,3}
            add_leaf();
            next_sym();
            if (sym != "LBRACE") {
//                error("'{'");
                error("array init");
            }
            add_leaf();
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
            add_leaf();
            init = true;
            SymTable::add(cur_func, tk2, var, dataType, local_addr, tmp_dim1, 0);
            if (cur_func != GLOBAL) {
                local_addr += size_of(dataType) * tmp_dim1;
            }
        } else if (sym == "LBRACK") {
            add_leaf();
            next_sym();
            UnsignedInt();
            tmp_dim2 = tk.v_int;
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            add_leaf();
            next_sym();
            if (sym == "ASSIGN") { //int a[3][3]={{1,2,3}, {4,5,6}, {7,8,9}}
                add_leaf();
                next_sym();
                if (sym != "LBRACE") {
//                    error("'{'");
                    error("array init");
                }
                add_leaf();
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
                    add_leaf();
                    do {
                        dim2_count++;
                        if (dim2_count > tmp_dim2) {
                            error("array init");
                        }
                        next_sym();
                        pair<DataType, string> con = Const();
                        add_2d_array(OP_ARR_SAVE, id, to_string(dim1_count - 1), to_string(dim2_count - 1), con.second,
                                     tmp_dim2);
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
                    add_leaf();
                    next_sym();
                } while (sym == "COMMA");
                //已预读
                if (dim1_count < tmp_dim1) {
                    error("array init");
                }
                if (sym != "RBRACE") {
                    error("'}'");
                }
                add_leaf();
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
                add_leaf();
                next_sym();
                UnsignedInt();
                tmp_dim1 = tk.v_int;
                next_sym();
                if (sym != "RBRACK") {
                    error("']'");
                }
                add_leaf();
                next_sym();
                if (sym == "LBRACK") {
                    add_leaf();
                    next_sym();
                    UnsignedInt();
                    tmp_dim2 = tk.v_int;
                    next_sym();
                    if (sym != "RBRACK") {
                        error("']'");
                    }
                    add_leaf();
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
    tree_backward();
}


void Grammar::TypeIdentifier() {
    if (sym != "INTTK" && sym != "CHARTK") {
        error("'int' or 'char'");
    }
    add_leaf();
}

void Grammar::SharedFuncDefBody() {
    int begin = pos - 1;
    if (sym != "LBRACE") {
        error("'{'");
    }
    add_leaf();
    next_sym();
    CompoundStmt();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    add_leaf();
    local_addr = LOCAL_ADDR_INIT;
    int end = pos - 1;
    if (mode == grammar_check && !para_assigned) {
        function_tokens_index.emplace_back(begin, end, cur_func);
    }
    para_assigned = false;

}

void Grammar::SharedFuncDefHead() {
    func_count++;
    if (sym != "LPARENT") {
        error("'('");
    }
    add_leaf();
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
    add_leaf();
}

void Grammar::RetFuncDef() {
    add_node("<有返回值函数定义>");
    DataType ret_type;
    if (sym == "INTTK") {
        ret_type = integer;
    } else if (sym == "CHARTK") {
        ret_type = character;
    } else {
        error("'int' or 'char'");
        ret_type = integer;
    }

    add_leaf();
    funcdef_ret = ret_type;

    add_node("<声明头部>");
    next_sym();
    Identifier();
    cur_func = tk.str;
    int idx = SymTable::add_func(tk, ret_type, tmp_paras);
    add_midcode(OP_FUNC, type_to_str(ret_type), tk.str, VACANT);
    output("<声明头部>");
    tree_backward();


    next_sym();
    SharedFuncDefHead();

    if (idx != -1) {
        SymTable::global[idx].paras = tmp_paras;
    }
    next_sym();
    SharedFuncDefBody();
    if (!has_returned) {
        error("ret return");
    }

    tmp_paras.clear();
    funcdef_ret = invalid_dt;
    has_returned = false;

    add_midcode(OP_END_FUNC, ret_type == integer ? "int" : "char", cur_func, VACANT);

    output("<有返回值函数定义>");
    tree_backward();
}

void Grammar::NonRetFuncDef() {
    add_node("<无返回值函数定义>");
    funcdef_ret = void_ret;
    if (sym != "VOIDTK") {
        error("'void");
    }
    add_leaf();

    next_sym();
    Identifier();
    cur_func = tk.str;
    int idx = SymTable::add_func(tk, void_ret, tmp_paras);
    add_midcode(OP_FUNC, "void", tk.str, VACANT);


    next_sym();
    SharedFuncDefHead();
    if (idx != -1) {
        SymTable::global[idx].paras = tmp_paras;
    }
    next_sym();
    SharedFuncDefBody();

    tmp_paras.clear();
    funcdef_ret = invalid_dt;
    has_returned = false;

    add_midcode(OP_RETURN, VACANT, VACANT, VACANT); //无返回值函数结尾补充return
    add_midcode(OP_END_FUNC, "void", cur_func, VACANT);

    output("<无返回值函数定义>");
    tree_backward();
}

void Grammar::CompoundStmt() {
    add_node("<复合语句>");
    if (mode != semantic_analyze) {
        if (sym == "CONSTTK") {
            ConstDeclare();
            next_sym();
        }
        if (sym == "INTTK" || sym == "CHARTK") {
            VariableDeclare();
            next_sym();
            //assert: 语句不以INTTK CHARTK开头
        }
    } else {
        while (sym == "CONSTTK" || sym == "INTTK" || sym == "CHARTK") {
            if (sym == "CONSTTK") {
                ConstDeclare();
                next_sym();
            }
            if (sym == "INTTK" || sym == "CHARTK") {
                VariableDeclare();
                next_sym();
                //assert: 语句不以INTTK CHARTK开头
            }
        }
    }
    StmtList();

    output("<复合语句>");
    tree_backward();
}

void Grammar::ParaList() {
    add_node("<参数表>");
    TypeIdentifier();
    DataType dataType = (sym == "INTTK") ? integer : character;
    next_sym();
    Identifier();
    tmp_paras.emplace_back(dataType, tk.str);
    SymTable::add(cur_func, tk, para, dataType, local_addr);
//    add_midcode(OP_PARA, type_to_str(dataType), tk.str, VACANT);
    local_addr += size_of(dataType);
    tmp_para_count = 1;
    next_sym();
    while (sym == "COMMA") {
        next_sym();
        TypeIdentifier();
        dataType = (sym == "INTTK") ? integer : character;
        add_leaf();
        next_sym();
        Identifier();
        tmp_paras.emplace_back(dataType, tk.str);
        SymTable::add(cur_func, tk, para, dataType, local_addr);
//        add_midcode(OP_PARA, type_to_str(dataType), tk.str, VACANT);
        local_addr += size_of(dataType);
        next_sym();
        tmp_para_count++;
    }

    output("<参数表>");
    retract();
    tree_backward();
    //空
}

void Grammar::Main() {
    func_count = 0;
    add_node("<主函数>");
    SymTable::add_func(tk, void_ret, tmp_paras);
    add_midcode(OP_FUNC, "void", "main", VACANT);
    funcdef_ret = void_ret;
    cur_func = "main";
    if (sym != "MAINTK") {
        error("'main'");
    }
    add_leaf();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    add_leaf();
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    add_leaf();
    next_sym();
    if (sym != "LBRACE") {
        error("'{'");
    }
    add_leaf();
    next_sym();
    CompoundStmt();
    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    add_leaf();
    add_midcode(OP_RETURN, VACANT, VACANT, VACANT); //无返回值函数结尾补充return
    add_midcode(OP_END_FUNC, "void", "main", VACANT);

    output("<主函数>");
    local_addr = LOCAL_ADDR_INIT;
    funcdef_ret = invalid_dt;
    tree_backward();
}

pair<DataType, string> Grammar::Expr() {
    add_node("<表达式>");
    DataType ret_type = invalid_dt;
    bool neg = false;
    if (sym == "PLUS" || sym == "MINU") {
        add_leaf();
        ret_type = integer;
        neg = sym == "MINU";
        next_sym();
    }
    pair<DataType, string> item = Item();
    string num1 = item.second;
    if (item.first == character && ret_type == invalid_dt) {
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
        add_leaf();
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
    if (ret_type == invalid_dt) {
        ret_type = integer;
    }
    tree_backward();
    return make_pair(ret_type, num1);
}

pair<DataType, string> Grammar::Item() {
    add_node("<项>");
    pair<DataType, string> factor = Factor();
    DataType ret_type = factor.first;
    string num1 = factor.second;
    next_sym();
    while (sym == "MULT" || sym == "DIV") {
        add_leaf();
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
    tree_backward();
    return make_pair(ret_type, num1);
}

pair<DataType, string> Grammar::Factor() {
    add_node("<因子>");
    DataType ret_type = integer;
    string ret_str;
    if (sym == "IDENFR") {
//        next_sym();
//        if (sym != "LPARENT") {
//            retract();
//        } else {
//            retract();
//        }
        add_leaf();
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
            in_factor = true;
            RetFuncCall();
            in_factor = false;
            ret_str = add_midcode(OP_ADD, "%RET", "0", AUTO);
            SymTable::add(cur_func, ret_str, tmp, ret_type, local_addr);
            local_addr += 4;
        } else if (sym == "LBRACK") {
            add_leaf();
            next_sym();
            pair<DataType, string> index1 = Expr();
            if (index1.first != integer) {
                error("array index type");
            }
            next_sym();
            if (sym != "RBRACK") {
                error("']'");
            }
            add_leaf();
            next_sym();
            if (sym == "LBRACK") {
                add_leaf();
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
                add_leaf();
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
        add_leaf();
        ret_type = integer;
        //ret_str = "(";
        next_sym();
        ret_str += Expr().second;
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        add_leaf();
        //ret_str += ")";
    } else if (sym == "PLUS" || sym == "MINU" || sym == "INTCON") {
        ret_type = integer;
        ret_str = Int();
    } else if (sym == "CHARCON") {
        add_leaf();
        ret_type = character;
        ret_str = "'" + tk.str + "'";
    } else {
        error("factor");
    }

    output("<因子>");
    tree_backward();
    return make_pair(ret_type, ret_str);
}

void Grammar::Stmt() {
    statement_begin_index = pos - 1;
    add_node("<语句>");
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
        add_leaf();
        next_sym();
        if (mode == semantic_analyze) {
            CompoundStmt();
        } else {
            StmtList();
        }
        next_sym();
        if (sym != "RBRACE") {
            error("'}'");
        }
        add_leaf();
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
    tree_backward();
}

void Grammar::AssignStmt() {
    add_node("<赋值语句>");
    string id = Identifier();
    string value;
    SymTableItem item = SymTable::search(cur_func, tk);
    if (item.valid && item.stiType == constant) {
        error("change const");
    } else if (item.stiType == para) {
        para_assigned = true;
    }
    next_sym();
    if (sym == "ASSIGN") {
        add_leaf();
        next_sym();
        value = Expr().second;
        add_midcode(OP_ASSIGN, id, value, VACANT);
    } else if (sym == "LBRACK") {
        add_leaf();
        next_sym();
        pair<DataType, string> index1 = Expr();
        if (index1.first != integer) {
            error("array index type");
        }
        next_sym();
        if (sym != "RBRACK") {
            error("']'");
        }
        add_leaf();
        next_sym();
        if (sym == "ASSIGN") {
            add_leaf();
            //a[2]=3
            next_sym();
            value = Expr().second;
            add_midcode(OP_ARR_SAVE, id, index1.second, value);
        } else if (sym == "LBRACK") {
            add_leaf();
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
            add_leaf();
            next_sym();
            if (sym != "ASSIGN") {
                error("'='");
            }
            add_leaf();
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
    tree_backward();
}

void Grammar::ConditionStmt() {
    add_node("<条件语句>");
    if (sym != "IFTK") {
        error("'if'");
    }
    add_leaf();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    add_leaf();
    next_sym();
    pair<string, string> cond = Condition();
    string label_else = add_midcode(OP_JUMP_IF, cond.first, cond.second, AUTO_LABEL);
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    add_leaf();
    next_sym();
    Stmt();
    next_sym();
    if (sym == "ELSETK") {
        add_leaf();
        string label_end = PseudoCodeList::assign_label();
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
    tree_backward();
}

pair<string, string> Grammar::Condition() {
    add_node("<条件>");
    pair<DataType, string> it1 = Expr();
    if (it1.first != integer) {
        error("condition type");
    }
    next_sym();
    //返回的第二个值是条件运算符的取反
    string op;
    if (sym == "LSS") {
        op = ">=0";
    } else if (sym == "LEQ") {
        op = ">0";
    } else if (sym == "GRE") {
        op = "<=0";
    } else if (sym == "GEQ") {
        op = "<0";
    } else if (sym == "EQL") {
        op = "!=0";
    } else if (sym == "NEQ") {
        op = "==0";
    } else {
        error("relation operator");
    }
    add_leaf();
    next_sym();
    pair<DataType, string> it2 = Expr();
    if (it2.first != integer) {
        error("condition type");
    }

    string ret = add_sub(it1.second, it2.second);

    output("<条件>");
    tree_backward();
    return make_pair(ret, op);
}

void Grammar::LoopStmt() {
    add_node("<循环语句>");
    if (sym == "WHILETK") {
        add_leaf();
        next_sym();
        if (sym != "LPARENT") {
            error("'('");
        }
        add_leaf();
        next_sym();


        int pos1 = pos - 1;
        pair<string, string> cond = Condition();
        int pos2 = pos;
        vector<Token> vec_cond(cur_lex_results.begin() + pos1, cur_lex_results.begin() + pos2);
        string label_end = add_midcode(OP_JUMP_IF, cond.first, cond.second, AUTO_LABEL);
        string label_begin = PseudoCodeList::assign_label();
        add_midcode(OP_LABEL, label_begin, VACANT, VACANT);
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        add_leaf();
        next_sym();
        Stmt();
        cur_lex_results.insert(cur_lex_results.begin() + pos,vec_cond.begin(), vec_cond.end());
        string op = cond.second == "==0" ? "!=0" :
                       cond.second == "!=0" ? "==0" :
                       cond.second == ">0" ? "<=0" :
                       cond.second == "<0" ? ">=0" :
                       cond.second == ">=0" ? "<0" :
                       cond.second == "<=0" ? ">0" : INVALID;
        next_sym();
        cond = Condition();
        add_midcode(OP_JUMP_IF, cond.first, op, label_begin);
        add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    } else if (sym == "FORTK") {
        add_leaf();
        next_sym();
        if (sym != "LPARENT") {
            error("'('");
        }
        add_leaf();
        next_sym();
        string id = Identifier();
        SymTable::search(cur_func, tk);
        SymTableItem item = SymTable::search(cur_func, tk);
        if (item.stiType == para) {
            para_assigned = true;
        }

        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        add_leaf();
        next_sym();
        string value = Expr().second;
        add_midcode(OP_ASSIGN, id, value, VACANT);
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        add_leaf();
        next_sym();
        int pos1 = pos - 1;
        pair<string, string> cond = Condition();
        int pos2 = pos;
        vector<Token> vec_cond(cur_lex_results.begin() + pos1, cur_lex_results.begin() + pos2);
        string label_end = add_midcode(OP_JUMP_IF, cond.first, cond.second, AUTO_LABEL);
        next_sym();
        if (sym != "SEMICN") {
            error("';'");
        }
        add_leaf();

        next_sym();
        id = Identifier();
        item = SymTable::search(cur_func, tk);
        if (item.stiType == para) {
            para_assigned = true;
        }
        next_sym();
        if (sym != "ASSIGN") {
            error("'='");
        }
        add_leaf();
        next_sym();
        string id2 = Identifier();
        SymTable::search(cur_func, tk);
        next_sym();
        string op = sym == "PLUS" ? OP_ADD : OP_SUB;
        if (sym != "PLUS" && sym != "MINU") {
            error("'+' or '-'");
        }
        add_leaf();
        next_sym();
        add_node("<步长>");
        UnsignedInt();
        string pace_length = tk.str;
        output("<步长>");
        tree_backward();

        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        add_leaf();
        string label_begin = PseudoCodeList::assign_label();
        add_midcode(OP_LABEL, label_begin, VACANT, VACANT);
        next_sym();
        Stmt();
        add_midcode(op, id2, pace_length, id);

        cur_lex_results.insert(cur_lex_results.begin() + pos,vec_cond.begin(), vec_cond.end());

        string op2 = cond.second == "==0" ? "!=0" :
                       cond.second == "!=0" ? "==0" :
                       cond.second == ">0" ? "<=0" :
                       cond.second == "<0" ? ">=0" :
                       cond.second == ">=0" ? "<0" :
                       cond.second == "<=0" ? ">0" : INVALID;
        next_sym();
        cond = Condition();
        add_midcode(OP_JUMP_IF, cond.first, op2, label_begin);
        add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    } else {
        error("'while' or 'for'");
    }

    output("<循环语句>");
    tree_backward();
}

void Grammar::CaseStmt() {
    add_node("<情况语句>");
    if (sym != "SWITCHTK") {
        error("'switch'");
    }
    add_leaf();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    add_leaf();
    next_sym();
    pair<DataType, string> expr = Expr();
    string expr_var = add_midcode(OP_ADD, expr.second, "0", AUTO_VAR);
    SymTable::add(cur_func, expr_var, var, expr.first, local_addr);
    add_midcode(OP_EMPTY, VACANT, VACANT, VACANT);
    //用来防止remove_redundant_assign时非tmp变量被优化掉
    local_addr += 4;
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    add_leaf();
    next_sym();
    if (sym != "LBRACE") {
        error("'{'");
    }
    add_leaf();
    next_sym();
    string label_end = PseudoCodeList::assign_label();

    add_node("<情况表>");
    add_node("<情况子语句>");
    if (sym != "CASETK") {
        error("'case'");
    }
    add_leaf();
    next_sym();
    pair<DataType, string> con = Const();
    if (con.first != expr.first) {
        error("const type");
    }
    string r = add_sub(expr_var, con.second);
    string label_next = add_midcode(OP_JUMP_IF, r, "!=0", AUTO_LABEL);
    next_sym();
    if (sym != "COLON") {
        error("':'");
    }
    add_leaf();
    next_sym();
    Stmt();
    add_midcode(OP_JUMP_UNCOND, label_end, VACANT, VACANT);
    output("<情况子语句>");
    tree_backward();

    next_sym();
    while (sym == "CASETK") {
        add_leaf();
        add_node("<情况子语句>");
        next_sym();
        con = Const();
        if (con.first != expr.first) {
            error("const type");
        }
        add_midcode(OP_LABEL, label_next, VACANT, VACANT);
        r = add_sub(expr_var, con.second);
        label_next = add_midcode(OP_JUMP_IF, r, "!=0", AUTO_LABEL);
        next_sym();
        if (sym != "COLON") {
            error("':'");
        }
        add_leaf();
        next_sym();
        Stmt();
        add_midcode(OP_JUMP_UNCOND, label_end, VACANT, VACANT);
        output("<情况子语句>");
        tree_backward();
        next_sym();
    }
    retract();
    output("<情况表>");
    tree_backward();

    add_node("<缺省>");
    next_sym();
    if (sym != "DEFAULTTK") {
        error("'default'");
        retract();
    } else {
        add_leaf();
        next_sym();
        if (sym != "COLON") {
            error("':'");
        }
        add_leaf();
        next_sym();
        add_midcode(OP_LABEL, label_next, VACANT, VACANT);
        Stmt();
    }
    output("<缺省>");
    tree_backward();

    next_sym();
    if (sym != "RBRACE") {
        error("'}'");
    }
    add_leaf();
    add_midcode(OP_LABEL, label_end, VACANT, VACANT);
    output("<情况语句>");
    tree_backward();
}

void Grammar::SharedFuncCall() {
    bool para_count_err = false;
    string func_name = Identifier();
    function_call_start_index = pos - 1;

    vector<pair<DataType, string>> paras = SymTable::search(cur_func, tk).paras;

    bool tokens_before_call = function_call_start_index > statement_begin_index;

    pair<int, int> index = make_pair(-1, -1);
    for (auto &ft: function_tokens_index) {
        if (ft.name == func_name) {
            index = make_pair(ft.begin, ft.end);
        }
    }

    in_factor = true; //内联开关

    if (!in_factor && cur_func != func_name &&
        mode == gen_inline && index.first != -1 && index.second != -1) {
        cout << "inline function: " << func_name << endl;
        DataType ret_type = SymTable::search_func(func_name).dataType;
        string ret_var;
        string prefix;
        ret_index++;
        for (int i = 0; i < ret_index; i++) {
            prefix += "&";
        }
        if (ret_type != void_ret) {
            ret_var += prefix + "RET" + prefix;
        }


        for (int i = statement_begin_index; i <= pos; i++) {
            //语句内函数调用前的词，挪到内联展开之后;函数调用的词(function_call_start_index和pos之间)直接删除
            new_lex_results.pop_back();
        }

        if (paras.empty()) {
            for (int i = index.first; i <= index.second; i++) {
                Token cur = cur_lex_results[i];
                if (cur.type == "RETURNTK") {
                    if (ret_type != void_ret) {
                        new_lex_results.emplace_back("IDENFR", ret_var);
                        new_lex_results.emplace_back("ASSIGN", "=");
                    }
                    continue;
                }
                SymTableItem symTableItem = SymTable::try_search(func_name, cur.str, false);
                if (cur.type == "IDENFR" && (SymTable::try_search(func_name, cur.str, false).stiType == var
                                             || SymTable::try_search(func_name, cur.str, false).stiType == constant)) {
                    new_lex_results.emplace_back("IDENFR", prefix + cur.str);
                    continue;
                }
                new_lex_results.push_back(cur);
                if (i == index.first && ret_type != void_ret) {
                    if (ret_type == integer) {
                        new_lex_results.emplace_back("INTTK", "int");
                    } else {
                        new_lex_results.emplace_back("CHARTK", "char");
                    }
                    new_lex_results.emplace_back("IDENFR", ret_var);
                    new_lex_results.emplace_back("SEMICN", ";");
                }
            }
            next_sym(false); // read (
            next_sym(false); // read )
        } else {
            vector<string> real_paras;
            next_sym(false); // read (

            int num_left_parent = 0;
            bool break_flag = false;
            next_sym(false);
            while (true) {
                string para;
                while (sym != "COMMA" || num_left_parent != 0) {
                    if (sym == "LPARENT") {
                        num_left_parent++;
                    } else if (sym == "RPARENT") {
                        if (num_left_parent == 0) {
                            break_flag = true;
                            break;
                        }
                        num_left_parent--;
                    }
                    if (sym == "CHARCON") {
                        para += "'" + tk.str + "'";
                    } else {
                        para += tk.str;
                    }
                    next_sym(false);
                }
                real_paras.push_back(para);
                if (break_flag) {
                    break;
                }
                next_sym(false);
            }

            for (int i = index.first; i <= index.second; i++) {
                Token cur = cur_lex_results[i];
                if (cur.type == "RETURNTK") {
                    if (ret_type != void_ret) {
                        new_lex_results.emplace_back("IDENFR", ret_var);
                        new_lex_results.emplace_back("ASSIGN", "=");
                    }
                    continue;
                }
                bool flag = false;
                for (int j = 0; j < paras.size(); j++) {
                    if (cur.str == paras[j].second) {
                        //替换形参为实参
                        Token new_token = cur;
                        new_token.str = real_paras[j];
                        new_lex_results.push_back(new_token);
                        flag = true;
                        break;
                    }
                }
                if (!flag) {
                    if (cur.type == "IDENFR" && (SymTable::try_search(func_name, cur.str, false).stiType == var
                                                 ||
                                                 SymTable::try_search(func_name, cur.str, false).stiType == constant)) {
                        new_lex_results.emplace_back("IDENFR", prefix + cur.str);
                        continue;
                    }
                    new_lex_results.push_back(cur);
                }
                if (i == index.first && ret_type != void_ret) {
                    if (ret_type == integer) {
                        new_lex_results.emplace_back("INTTK", "int");
                    } else {
                        new_lex_results.emplace_back("CHARTK", "char");
                    }
                    new_lex_results.emplace_back("IDENFR", ret_var);
                    new_lex_results.emplace_back("SEMICN", ";");
                }
            }
        }

        for (int i = statement_begin_index; i < function_call_start_index; i++) {
            new_lex_results.push_back(cur_lex_results[i]);
        }

        if (tokens_before_call) {
            //&RET&代替函数调用
            new_lex_results.emplace_back("IDENFR", ret_var);
        }

        return;
    }

    add_midcode(OP_PREPARE_CALL, func_name, VACANT, VACANT);

    pair<DataType, string> expr;
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    add_leaf();
    next_sym();
    if (sym == "RPARENT") {
        retract();
        output("<值参数表>");
    } else {
        if (paras.begin() == paras.end()) {
//            error("para count");
            para_count_err = true;
            expr = Expr();
            add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
        } else {
            expr = Expr();
            if (paras.begin()->first != expr.first) {
                error("para type");
            }
            paras.erase(paras.begin());
            add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
        }
        next_sym();
        while (sym == "COMMA") {
            next_sym();
            if (paras.begin() == paras.end()) {
                para_count_err = true;
                expr = Expr();
                add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
            } else {
                expr = Expr();
                if (paras.begin()->first != expr.first) {
                    error("para type");
                }
                paras.erase(paras.begin());
                add_midcode(OP_PUSH_PARA, expr.second, VACANT, VACANT);
            }
            next_sym();
        }
        output("<值参数表>");
        retract();
        //空
    }
    next_sym();
    if (!paras.empty()) {
        para_count_err = true;
    }
    if (sym != "RPARENT") {
        error("')'");
    } else if (para_count_err) {
        error("para count");
    }
    add_leaf();


    add_midcode(OP_CALL, func_name, VACANT, VACANT);
}

void Grammar::RetFuncCall() {
    add_node("<有返回值函数调用语句>");
    SharedFuncCall();
    output("<有返回值函数调用语句>");
    tree_backward();
}

void Grammar::NonRetFuncCall() {
    add_node("<无返回值函数调用语句>");
    SharedFuncCall();
    output("<无返回值函数调用语句>");
    tree_backward();
}

void Grammar::StmtList() {
    add_node("<语句列>");
    while (sym == "WHILETK" || sym == "FORTK" || sym == "SWITCHTK" ||
           sym == "IFTK" || sym == "SCANFTK" || sym == "PRINTFTK" ||
           sym == "RETURNTK" || sym == "LBRACE" || sym == "IDENFR" ||
           sym == "SEMICN") {
        Stmt();
        next_sym();
    }

    output("<语句列>");
    retract();
    tree_backward();
    //空
}

void Grammar::ReadStmt() {
    add_node("<读语句>");
    if (sym != "SCANFTK") {
        error("'scanf'");
    }
    add_leaf();
    next_sym();
    if (sym != "LPARENT") {
        error("'('");
    }
    add_leaf();
    next_sym();
    Identifier();
    add_midcode(OP_SCANF, tk.str, VACANT, VACANT);
    SymTableItem item = SymTable::search(cur_func, tk);
    if (item.stiType == constant) {
        error("change const");
    }
    if (item.stiType == para) {
        para_assigned = true;
    }
    next_sym();
    if (sym != "RPARENT") {
        error("')'");
    }
    add_leaf();

    output("<读语句>");
    tree_backward();
}

void Grammar::WriteStmt() {
    add_node("<写语句>");
    if (sym != "PRINTFTK") {
        error("'printf'");
    }
    add_leaf();
    next_sym();
    if (sym != "LPARENT") {
        error("'(");
    }
    add_leaf();
    next_sym();
    if (sym == "STRCON") {
        add_leaf();
        add_node("<字符串>");
        PseudoCodeList::strcons.push_back(str_replace(tk.str, "\\", "\\\\"));
        add_midcode(OP_PRINT, to_string(PseudoCodeList::strcons.size() - 1), "strcon", VACANT);
        output("<字符串>");
        tree_backward();
        next_sym();
        if (sym == "COMMA") {
            next_sym();
            pair<DataType, string> p = Expr();
            add_midcode(OP_PRINT, p.second, p.first == character ? "char" : "int", VACANT);
            next_sym();
            if (sym != "RPARENT") {
                error("')'");
            }
            add_leaf();
        } else {
            if (sym != "RPARENT") {
                error("')'");
            }
            add_leaf();
        }
    } else {
        pair<DataType, string> p = Expr();
        add_midcode(OP_PRINT, p.second, p.first == character ? "char" : "int", VACANT);
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        add_leaf();
    }
    add_midcode(OP_PRINT, ENDL, VACANT, VACANT);

    output("<写语句>");
    tree_backward();
}

void Grammar::ReturnStmt() {
    add_node("<返回语句>");
    if (sym != "RETURNTK") {
        error("'return'");
    }
    add_leaf();
    next_sym();
    if (sym == "LPARENT") {
        add_leaf();
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
        next_sym();
        if (sym != "RPARENT") {
            error("')'");
        }
        add_leaf();
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
    tree_backward();
}

string Grammar::add_midcode(const string &op, const string &n1, const string &n2, const string &r) const {
    string num1 = const_replace(n1);
    string num2 = const_replace(n2);
    string result = r;
    if (op == OP_ARR_SAVE) {
        result = const_replace(r);
    }
    return PseudoCodeList::add(op, num1, num2, result);
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

string Grammar::add_sub(const string &num1, const string &num2) {
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

string Grammar::add_2d_array(const string &op, const string &n1, const string &n2, const string &n3, const string &r,
                             int dim2_size) {
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

void Grammar::tree_backward() {
    cur_node = nodes[cur_node].parent;
}

void Grammar::add_node(const string &name) {
    nodes.emplace_back(name, INVALID, cur_node);
    nodes[cur_node].child.push_back(nodes.size() - 1);
    cur_node = nodes.size() - 1;
}

void Grammar::add_leaf() {
    nodes.emplace_back(tk.str, tk.type, cur_node);
    nodes[cur_node].child.push_back(nodes.size() - 1);
    cur_node = nodes.size() - 1;
    tree_backward();
}

void Grammar::dfs_show(const TreeNode &node, int depth) {
    for (int i = 0; i < depth - 1; i++) {
        cout << "|   ";
    }
    if (depth != 0) {
        cout << "|---";
    }
    cout << (node.type == INVALID ? "" : node.type) << " " << node.str << endl;
    for (auto &child: node.child) {
        dfs_show(nodes[child], depth + 1);
    }
}

void Grammar::show_tree() {
//    for (auto &node: nodes) {
//        if (node.child.empty()) {
//            continue;
//        }
//        cout << (node.type == INVALID ? "" : node.type) << " " << node.str
//             << "  Parent: " << nodes[node.parent].str << "  Children: ";
//        for (int child: node.child) {
//            cout << nodes[child].str << " ";
//        }
//        cout << endl;
//    }
    dfs_show(nodes[0], 0);
}

void Grammar::save_lexer_results(const string &path) {
    ofstream out(path);
    if (mode == grammar_check) {
        new_lex_results = cur_lex_results;
    }
    for (auto &token: new_lex_results) {
        if (token.type == "STRCON") {
            out << "\"" << token.str << "\"";
        } else if (token.type == "CHARCON") {
            out << "\'" << token.str << "\'";
        } else {
            out << token.str;
        }
        if (token.type == "SEMICN" || token.type == "LBRACE") {
            out << "\n";
        } else if (token.type == "RBRACE") {
            out << "\n\n";
        } else if (token.type != "LPARENT" && token.type != "RPARENT") {
            out << " ";
        }
    }
    out.close();
}

