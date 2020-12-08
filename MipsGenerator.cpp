#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-inefficient-string-concatenation"
//
// Created by wzk on 2020/11/5.
//

#include "MipsGenerator.h"

#include <utility>

//TODO：用a寄存器且不区分ast；sp直接硬编码；

void MipsGenerator::generate(const string &code) {
    mips.push_back(code);
    if (DEBUG) {
        cout << code << endl;
    }
}

void MipsGenerator::generate(const string &op, const string &num1) {
    generate(op + " " + num1);
}

void MipsGenerator::generate(const string &op, const string &num1, const string &num2) {
    generate(op + " " + num1 + ", " + num2);
    release(num2);
    if (op == "sw" || op == "bltz" || op == "blez" || op == "bgtz" || op == "bgez") {
        release(num1);
    }
}

void MipsGenerator::generate(const string &op, const string &num1, const string &num2, const string &num3) {
    generate(op + " " + num1 + ", " + num2 + ", " + num3);
    if (op == "addu" || op == "subu" || op == "addiu"
        || op == "mul" || op == "div" || op == "sll" || op == "sra") {
        if (num1 != num2) {
            release(num2);
        }
        if (num1 != num3) {
            release(num3);
        }
    } else if (op == "beq" || op == "bne") {
        release(num1);
    }
}

void MipsGenerator::release(string addr) {
    for (auto &item: t_regs) {
        if (item == addr) {
            t_reg_table[addr[2] - '0'] = VACANT;
            generate("# RELEASE " + addr);
            return;
        }
    }
}

void MipsGenerator::translate() {
    generate(".data");
    for (auto &item: SymTable::global) {
        if (item.dim >= 1) {
            generate("arr__" + item.name + "_: .space " + to_string(item.size));
        }
    }
    for (auto &it: SymTable::local) {
        if (SymTable::search_func(it.first).recur_func) {
//            cout << "recursive function: " << it.first << endl;
            continue;
        }
        for (auto &item: it.second) {
            if (item.dim >= 1) {
                generate("arr_" + it.first + "_" + item.name + "_: .space " + to_string(item.size + 4));
            }
        }
    }

    for (int i = 0; i < strcons.size(); i++) {
        if (strcons[i][0] != '@') {
            generate("str__" + to_string(i) + ": .asciiz \"" + strcons[i] + "\"");
        } else {
            generate("str__" + to_string(i) + ": .ascii \"" + strcons[i].substr(1) + "\"");
        }
    }
    generate(R"(newline__: .asciiz "\n")");
    generate(".text");

    bool init = true;

    vector<string> s_old;

    int idx = 0;

    for (auto &code:mid) {
        generate("");
        generate("# === " + code.to_str() + " ===");
        //generate("# === " + to_string(idx) + " " + code.to_str() + " ===");
        idx++;
        string op = code.op, num1 = code.num1, num2 = code.num2, result = code.result;
        if (op == OP_FUNC) {
            //进入新函数

            if (init) {
                //此前为全局变量初始化，在此调用主函数
                generate("addi $sp, $sp, -" + to_string(LOCAL_ADDR_INIT + SymTable::func_size("main")));
                generate("j main");
                init = false;
            }

            cur_func = num2;
            call_func_sp_offset = 0;
            generate(num2 + ":");
            //被调用者保护s
            for (int i = 0; i < NUM_S_REG; i++) {
                s_reg_table[i] = VACANT;
            }
            for (auto &it: SymTable::search_func(cur_func).paras) {
                string sreg = assign_s_reg(it.second);
                if (sreg != INVALID) {
                    generate("lw", sreg, to_string(SymTable::search(cur_func, it.second).addr
                                                   + call_func_sp_offset) + "($sp)");
                } else {
                    break;
                }
            }

        } else if (op == OP_RETURN) {
            //恢复s
            if (num1 != VACANT) {
                load_value(num1, "$v0");
            }
            if (cur_func == "main") {
                generate("li $v0, 10");
                generate("syscall");
            } else {
                generate("jr $ra");
            }
        } else if (op == OP_PREPARE_CALL) {
            sp_size.push_back(LOCAL_ADDR_INIT + SymTable::func_size(num1));
            call_func_sp_offset = sum(sp_size);
            generate("addi $sp, $sp, -" + to_string(sp_size.back()));
            call_func_paras.push_back(SymTable::local[num1]);
        } else if (op == OP_PUSH_PARA) {
            /*
             * a在内存，b在寄存器：sw b,a
             * a在内存，b在内存：lw reg,b  sw reg,a
             * a在内存，b为常量 li reg,b sw reg,a
             *
             * a在寄存器，b在寄存器：move b,a
             * a在寄存器，b在内存：lw a,b
             * a在寄存器，b为常量 li a,b
             */

            string para_addr = to_string(call_func_paras.back().begin()->addr) + "($sp)";
            assertion(call_func_paras.back().begin()->stiType == para);
            call_func_paras.back().erase(call_func_paras.back().begin());
            string reg = "$a1";


            bool b_in_reg = in_reg(num1) || assign_reg(num1, true);
            string b = symbol_to_addr(num1);
            if (b_in_reg) {
                generate("sw", b, para_addr);
            } else if (is_const(num1)) {
                generate("li", reg, b);
                generate("sw", reg, para_addr);
            } else {
                generate("lw", reg, b);
                generate("sw", reg, para_addr);
            }

        } else if (op == OP_CALL) {
            //调用者保护t
            show_reg_status();
            vector<int> saved_s, saved_t;
            vector<string> t_old = t_reg_table;
            for (int i = 0; i < NUM_T_REG; i++) {
                if (t_reg_table[i] != VACANT) {
                    saved_t.push_back(i);
                    generate("sw", t_regs[i],to_string(STACK_T_BEGIN + 4 * i) + "($sp)");
                    t_reg_table[i] = VACANT;
                }
            }

            s_old = s_reg_table;
            for (int i = 0; i < NUM_S_REG; i++) {
                if (s_reg_table[i] != VACANT) {
                    saved_s.push_back(i);
                    generate("sw", s_regs[i], to_string(STACK_S_BEGIN + 4 * i) + "($sp)");
                    s_reg_table[i] = VACANT;
                }
            }
            if (cur_func != "main") {
                generate("sw", "$ra", STACK_RA);
            }
            assertion(call_func_paras.back().empty() || call_func_paras.back().begin()->stiType != para);
            generate("jal", num1);

            if (cur_func != "main") {
                generate("lw", "$ra", STACK_RA);
            }
            for (int i: saved_s) {
                generate("lw", s_regs[i],to_string(STACK_S_BEGIN + 4 * i) + "($sp)");
            }
            s_reg_table = s_old;

            //调用者恢复t
            for (int i: saved_t) {
                generate("lw", t_regs[i],to_string(STACK_T_BEGIN + 4 * i) + "($sp)");
            }
            t_reg_table = t_old;
            if (sp_size.back() != 0) {
                generate("addi $sp, $sp, " + to_string(sp_size.back()));
                sp_size.pop_back();
            }

            call_func_sp_offset = sum(sp_size);
            call_func_paras.pop_back();
        } else if (op == OP_END_FUNC) {
//            show_reg_status();
        } else if (op == OP_LABEL) {
            generate(num1 + ":");
        } else if (op == OP_PRINT) {
            if (num2 == "strcon") {
                generate("la $a0, str__" + num1);
                generate("li $v0, 4");
                generate("syscall");
            } else if (num1 == ENDL) {
                generate("la $a0, newline__");
                generate("li $v0, 4");
                generate("syscall");
            } else {
                //PRINT 表达式
                load_value(num1, "$a0");

                if (num2 == "int") {
                    generate("li $v0, 1");
                } else {
                    generate("li $v0, 11");
                }
                generate("syscall");
            }

        } else if (op == OP_SCANF) {
            SymTableItem it = SymTable::search(cur_func, num1);
            if (it.dataType == integer) {
                generate("li $v0, 5");
            } else {
                generate("li $v0, 12");
            }
            generate("syscall");
            save_value("$v0", num1);
        } else if (op == OP_ASSIGN) {
            gen_assign(num1, num2);
        } else if (is_arith(op) || op == OP_SRA) {
            /* 对于a=b+c:
             * abc都在寄存器/常量：                add a,b,c
             * ab在寄存器/常量，c在内存（或反过来）： lw reg2,c  add a,b,reg2
             * a在寄存器，bc在内存：               lw reg1,b  lw reg2,c  add a,reg1,reg2
             * a在内存，bc在寄存器/常量：           add reg1,b,c  sw reg1,a
             * ab在内存，c在寄存器/常量（或反过来）： lw reg1,b  add reg1,reg1,c  sw reg1,a
             * abc都在内存：                     lw reg1,b  lw reg2,c  add reg1,reg1,reg2  sw reg1,a
             */

            string instr = op_to_instr.find(op)->second;
            string reg1 = "$a1";
            string reg2 = "$a2";
            bool a_in_reg = in_reg(result) || assign_reg(result);
            //only_para：防止先被存到内存后被分配寄存器的情况
            bool b_in_reg_or_const = is_const(num1) || in_reg(num1) || assign_reg(num1, true);
            bool c_in_reg_or_const = is_const(num2) || in_reg(num2) || assign_reg(num2, true);

            string a = symbol_to_addr(result);
            string b = symbol_to_addr(num1);
            string c = symbol_to_addr(num2);

            bool is_2_pow_1 = is_const(code.num1) && is_2_power(stoi(code.num1));
            bool is_2_pow_2 = is_const(code.num2) && is_2_power(stoi(code.num2));
            bool sra = false;
            if ((op == OP_MUL || op == OP_DIV) && optimize_muldiv) {
                if (is_2_pow_1 && op == OP_MUL) {
                    // mul a, 8, c : sll a, c, 3
                    instr = "sll";
                    b = c;
                    c = to_string(int(log2(stoi(code.num1))));
                    b_in_reg_or_const = c_in_reg_or_const;
                    c_in_reg_or_const = true;
                } else if (is_2_pow_2 && op == OP_MUL) {
                    // mul a, b, 8 : sll a, b, 3
                    instr = "sll";
                    c = to_string(int(log2(stoi(code.num2))));
                } else if (is_2_pow_2 && op == OP_DIV) {
                    // mul a, b, 8 : sll a, b, 3
                    instr = "sra";
                    c = to_string(int(log2(stoi(code.num2))));
                }
            }

            if (a_in_reg) {
                if (b_in_reg_or_const && c_in_reg_or_const) {
                    gen_arithmetic(instr, a, b, c);
                } else if (b_in_reg_or_const) {
                    generate("lw", reg1, c);
                    gen_arithmetic(instr, a, b, reg1);
                } else if (c_in_reg_or_const) {
                    generate("lw", reg1, b);
                    gen_arithmetic(instr, a, reg1, c);
                } else {
                    generate("lw", reg1, b);
                    generate("lw", reg2, c);
                    gen_arithmetic(instr, a, reg1, reg2);
                }
            } else {
                if (b_in_reg_or_const && c_in_reg_or_const) {
                    gen_arithmetic(instr, reg1, b, c);
                    generate("sw", reg1, a);
                } else if (b_in_reg_or_const) {
                    generate("lw", reg1, c);
                    gen_arithmetic(instr, reg1, b, reg1);
                    generate("sw", reg1, a);
                } else if (c_in_reg_or_const) {
                    generate("lw", reg1, b);
                    gen_arithmetic(instr, reg1, reg1, c);
                    generate("sw", reg1, a);
                } else {
                    generate("lw", reg1, b);
                    generate("lw", reg2, c);
                    gen_arithmetic(instr, reg1, reg1, reg2);
                    generate("sw", reg1, a);
                }
            }

        } else if (op == OP_ARR_LOAD || op == OP_ARR_SAVE) {
            /* 对于a=b[c]:
             * load c到reg  sll reg,reg,2
             * a在寄存器，c为全局数组：lw a,b(reg)
             * a在内存，c为全局数组： lw reg2,b(reg)  sw reg2, a
             * a在寄存器，c为局部数组：add reg,reg,offset  add reg,reg,$sp  lw a,0(reg)
             * a在内存，c为局部数组： add reg,reg,offset  add reg,reg,$sp  lw reg2,reg($sp)  sw reg2, a
             */

            /* 对于b[c]=a:
             * load c到reg  sll reg,reg,2
             * a在寄存器，b为全局数组：sw a,b(reg)
             * a在内存，b为全局数组： lw reg2,a  sw reg2,b(reg)
             * a在寄存器，b为局部数组：add reg,reg,offset  sw a,b(reg)
             * a在内存，b为局部数组： add reg,reg,offset  lw reg2,a  sw reg2,reg($sp)
             * a为常量，b为全局数组：li reg2,a  sw a,b(reg)
             * a为常量，b为局部数组：add reg,reg,offset  li reg2,a  sw reg2,reg($sp)
             */

            string symbol = "^" + num1 + "[" + num2 + "]";

            bool a_in_reg = in_reg(result) || assign_reg(result, op == OP_ARR_SAVE);
            string a = symbol_to_addr(result);
            string reg = "$a1";
            string reg2 = "$a2";
            string item_addr;

            bool array_in_global = SymTable::in_global(cur_func, num1);

            if (begins_num(num2)) {     //下标是常数，可以简化计算
                int offset = 4 * stoi(num2);
                if (array_in_global) {
                    item_addr = "arr__" + num1 + "_+" + to_string(offset) + "($zero)";
                } else if (!SymTable::search_func(cur_func).recur_func) {
                    item_addr = "arr_" + cur_func + "_" + num1 + "_+" + to_string(offset) + "($zero)";
                } else {
                    offset += SymTable::search(cur_func, num1).addr + call_func_sp_offset;
                    item_addr = to_string(offset) + "($sp)";
                }
            } else {    //下标是变量，在内存或寄存器，4*num2+sp+call_func_sp_offset
                bool index_in_reg = in_reg(num2) || assign_reg(num2, true);
                string index = symbol_to_addr(num2);
                if (index_in_reg) {
                    generate("sll", reg, index, "2");
                } else {
                    generate("lw", reg, index);
                    generate("sll", reg, reg, "2");
                }

                if (array_in_global) {
                    item_addr = "arr__" + num1 + "_(" + reg + ")";
                } else if (!SymTable::search_func(cur_func).recur_func) {
                    item_addr = "arr_" + cur_func + "_" + num1 + "_(" + reg + ")";
                } else {
                    generate("addu", reg, reg, to_string(SymTable::search(cur_func, num1).addr + call_func_sp_offset));
                    generate("addu", reg, reg, "$sp");
                    item_addr = "0(" + reg + ")";
                }
            }


            if (op == OP_ARR_LOAD) {
                if (a_in_reg) {
                    generate("lw", a, item_addr);
                } else {
                    generate("lw", reg2, item_addr);
                    generate("sw", reg2, a);
                }
            } else {
                if (a_in_reg) {
                    generate("sw", a, item_addr);
                } else if (is_const(result)) {
                    generate("li", reg2, a);
                    generate("sw", reg2, item_addr);
                } else {
                    generate("lw", reg2, a);
                    generate("sw", reg2, item_addr);
                }
            }
        } else if (op == OP_JUMP_UNCOND) {
            generate("j", num1);
        } else if (op == OP_JUMP_IF) {
            if (begins_num(num1)) {
                int v = stoi(num1);
                bool jump = num2 == "<=0" ? v <= 0 : num2 == ">=0" ? v >= 0 : num2 == "==0" ? v == 0 :
                                                                              num2 == ">0" ? v > 0 : num2 == "<0" ? v <
                                                                                                                    0 :
                                                                                                     num2 == "!=0" ?
                                                                                                     v != 0 : true;
                if (jump) {
                    generate("j", result);
                } else {
                    //always not jump; ignore
                }
                continue;
            }
            bool a_in_reg = in_reg(num1) || assign_reg(num1, true);
            string a = symbol_to_addr(num1);
            string reg = "$a1";
            if (a_in_reg) {
                reg = a;
            } else if (is_const(num1)) {
                generate("li", reg, a);
            } else {
                generate("lw", reg, a);
            }
            if (num2 == "<0") {
                generate("bltz", reg, result);
            } else if (num2 == "<=0") {
                generate("blez", reg, result);
            } else if (num2 == ">0") {
                generate("bgtz", reg, result);
            } else if (num2 == ">=0") {
                generate("bgez", reg, result);
            } else if (num2 == "==0") {
                generate("beq", reg, "$zero", result);
            } else if (num2 == "!=0") {
                generate("bne", reg, "$zero", result);
            } else {
                panic("unknown operator in jump_if: " + num2);
            }
        }
    }
}

void MipsGenerator::gen_arithmetic(const string &instr, const string &num1, const string &num2, const string &num3) {
    string reg3 = "$a3";
    if (is_const(num2)) {
        if (instr == "addu") {
            generate("addiu", num1, num3, num2);
        } else if (instr == "mul") {
            generate(instr, num1, num3, num2);
        } else if (instr == "div" || instr == "subu") {
            //div  a,5,b:  li reg3,5  divu a,reg3,b

            generate("li", reg3, num2);
            if (instr == "div" && optimize_muldiv) {
                generate("div", reg3, num3);
                generate("mflo", num1);
                if (num1 != num3) {
                    release(num3);
                }
            } else {
                generate(instr, num1, reg3, num3);
            }
        } else {
            generate(instr, num1, num2, num3);
        }
    } else if (instr == "div" && !is_const(num3) && optimize_muldiv) {
        generate("div", num2, num3);
        generate("mflo", num1);
        if (num1 != num2) {
            release(num2);
        }
        if (num1 != num3) {
            release(num3);
        }
    } else if (instr == "sra" || instr == "srav") {
        string label = assign_label();
        string label2 = assign_label();
        generate("bgez", num2, label);
        generate("subu", reg3, "$zero", num2);
        generate(instr, num1, reg3, num3);
        generate("subu", num1, "$zero", num1);
        generate("j", label2);
        generate(label + ":");
        generate(instr, num1, num2, num3);
        generate(label2 + ":");
    } else if (instr == "addu" && is_const(num3)) {
        if (num3 == "1073741824") {
            generate("lui", reg3, "0x4000");
            generate("addu", num1, num2, reg3);
        } else {
            generate("addiu", num1, num2, num3);
        }
    } else if (instr == "subu" && is_const(num3)) {
        if (num3 == "1073741824") {
            generate("lui", reg3, "0xc000");
            generate("addu", num1, num2, reg3);
        } else {
            string neg = num3[0] == '+' ? '-' + num3.substr(1)
                                        : num3[0] == '-' ? num3.substr(1) : '-' + num3;
            generate("addiu", num1, num2, neg);
        }
    } else {
        generate(instr, num1, num2, num3);
    }
}


void MipsGenerator::gen_assign(const string &num1, const string &num2) {
    /* 对于a=b:
     * a在寄存器，b在寄存器：move a,b
     * a在寄存器，b在内存：lw a,b
     * a在寄存器，b为常量：li a,b
     *
     * a在内存，b在寄存器：sw b,a
     * a在内存，b在内存：lw reg,b  sw reg,a
     * a在内存，b为常量 li reg,b sw reg,a
     */

    bool a_in_reg = in_reg(num1) || assign_reg(num1);

    string a = symbol_to_addr(num1);

    string reg = "$a1";

    if (a_in_reg) {
        load_value(num2, a);
    } else {
        bool b_in_reg = in_reg(num2) || assign_reg(num2, true);
        string b = symbol_to_addr(num2);
        if (b_in_reg) {
            generate("sw", b, a);
        } else if (is_const(num2)) {
            generate("li", reg, b);
            generate("sw", reg, a);
        } else {
            generate("lw", reg, b);
            generate("sw", reg, a);
        }
    }
}


//将symbol的值读到对应寄存器
void MipsGenerator::load_value(const string &symbol, const string &reg) {
    bool inreg = in_reg(symbol) || assign_reg(symbol, true);
    string addr = symbol_to_addr(symbol);
    if (inreg) {
        generate("move", reg, addr);
    } else if (is_const(symbol)) {
        generate("li", reg, addr);
    } else {
        generate("lw", reg, addr);
    }
}

//将reg的值存到symbol的位置
void MipsGenerator::save_value(const string &reg, const string &symbol) {
    bool inreg = in_reg(symbol) || assign_reg(symbol);
    string addr = symbol_to_addr(symbol);
    if (inreg) {
        generate("move " + addr + ", " + reg);
    } else if (!is_const(symbol)) {
        generate("sw " + reg + ", " + addr);
    } else {
        panic(symbol + "not in memory or reg");
    }
}

bool MipsGenerator::assign_reg(const string &symbol, bool only_para) {
    if (!SymTable::in_global(cur_func, symbol) && optimize_assign_reg && !is_const(symbol)) {
        SymTableItem item = SymTable::search(cur_func, symbol);
        if (item.stiType == var && !only_para) {
            string sreg = assign_s_reg(symbol);
            if (sreg != INVALID) {
                return true;
            }
        }

        else if (item.stiType == tmp && !only_para) {
            string treg = assign_t_reg(symbol);
            if (treg != INVALID) {
                return true;
            }
        }
    }
    return false;
}

bool MipsGenerator::in_reg(const string &symbol) {
    if (symbol == "0" || symbol == "%RET") {
        return true;
    }
    if (is_const(symbol)) {
        return false;
    }
    for (int i = 0; i < NUM_T_REG; i++) {
        if (t_reg_table[i] == symbol) {
            return true;
        }
    }
    for (int i = 0; i < NUM_S_REG; i++) {
        if (s_reg_table[i] == symbol) {
            return true;
        }
    }

    return false;
}

bool MipsGenerator::in_memory(const string &symbol) {
    return !is_const(symbol) && !in_reg(symbol);
}

bool MipsGenerator::is_const(const string &symbol) {
    return begins_num(symbol) && symbol != "0";
}

//返回symbol对应的寄存器或地址，或常量值
string MipsGenerator::symbol_to_addr(const string &symbol) {
    if (begins_num(symbol)) {
        if (symbol == "0") {
            return "$zero"; //0寄存器
        }
        return symbol; //int
    }

    if (symbol == "%RET") {
        return "$v0";
    }

    for (int i = 0; i < NUM_T_REG; i++) {
        if (t_reg_table[i] == symbol) {
            return t_regs[i];
        }
    }
    for (int i = 0; i < NUM_S_REG; i++) {
        if (s_reg_table[i] == symbol) {
            return s_regs[i];
        }
    }

    SymTableItem item = SymTable::search(cur_func, symbol);
    if (SymTable::in_global(cur_func, symbol)) { //symbol不是局部变量
        SymTableItem global = SymTable::try_search(cur_func, symbol, true);
        if (global.valid && (global.stiType == var || global.stiType == tmp)) {
            return to_string(item.addr - LOCAL_ADDR_INIT) + "($gp)";
        }
    }
    return to_string(item.addr + call_func_sp_offset) + "($sp)";
}

string MipsGenerator::assign_t_reg(const string &name) {
    for (int i = 0; i < NUM_T_REG; i++) {
        if (t_reg_table[i] == VACANT) {
            t_reg_table[i] = name;
            return t_regs[i];
        }
    }
    return INVALID;
}

string MipsGenerator::assign_s_reg(const string &name) {
    for (int i = 0; i < NUM_S_REG; i++) {
        if (s_reg_table[i] == VACANT) {
            s_reg_table[i] = name;
            return s_regs[i];
        }
    }

    return INVALID;
}

void MipsGenerator::show_reg_status() {
    cout << "==========REG TABLE==========" << endl;
    for (int i = 0; i < NUM_T_REG; i += 2) {
        cout << t_regs[i] << ": " << t_reg_table[i] << "   ";
        cout << t_regs[i + 1] << ": " << t_reg_table[i + 1] << endl;
    }
    for (int i = 0; i < NUM_S_REG; i += 2) {
        cout << s_regs[i] << ": " << s_reg_table[i] << "    ";
        cout << s_regs[i + 1] << ": " << s_reg_table[i + 1] << endl;
    }
    cout << "=============================" << endl;
}

#pragma clang diagnostic pop