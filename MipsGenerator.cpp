#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-inefficient-string-concatenation"
//
// Created by wzk on 2020/11/5.
//

#include "MipsGenerator.h"

#include <utility>

//TODO：分配st寄存器

void MipsGenerator::generate(const string &code) {
    mips.push_back(code);
}

void MipsGenerator::translate() {
    generate(".data");
    for (int i = 0; i < strcons.size(); i++) {
        generate("str" + to_string(i) + ": .asciiz \"" + strcons[i] + "\"");
    }
    generate(R"(newline: .asciiz "\n")");

    generate(".text");
//    generate("lui $gp, 0x1001");
    generate("jal main");
    generate("li $v0, 10");
    generate("syscall");

    for (auto &code:mid) {
        generate("# === " + code.to_str() + " ===");
        string op = code.op;
        if (op == OP_FUNC) {
            //进入新函数
            for (int i = 0; i < 8; i++) {
                s_reg_table[i] = VACANT;
            }
            if (sp_size != 0) {
                generate("addi $sp, $sp, " + to_string(sp_size));
            }

            cur_func = code.num2;
            generate(code.num2 + ":");
            if (!SymTable::local[code.num2].empty()) {
                sp_size = SymTable::local[code.num2].back().addr + SymTable::local[code.num2].back().size;
                generate("addi $sp, $sp, -" + to_string(sp_size));
            } else {
                sp_size = 0;
            }
        } else if (op == OP_PRINT) {
            if (code.num2 == "strcon") {
                generate("la $a0, str" + code.num1);
                generate("li $v0, 4");
                generate("syscall");
            } else if (code.num1 == ENDL) {
                generate("la $a0, newline");
                generate("li $v0, 4");
                generate("syscall");
            } else {
                //PRINT 表达式
                SymTableItem it = SymTable::try_search(cur_func, code.num1, true);
                load_value(code.num1, "$a0");
                if (begins_num(code.num1) || (it.valid && it.dataType == integer)) {
                    generate("li $v0, 1");
                } else {
                    generate("li $v0, 11");
                }
                generate("syscall");
            }

        } else if (op == OP_SCANF) {
            SymTableItem it = SymTable::search(cur_func, code.num1);
            if (it.dataType == integer) {
                generate("li $v0, 5");
            } else {
                generate("li $v0, 12");
            }
            generate("syscall");
            save_value("$v0", code.num1);
        } else if (op == OP_ASSIGN) {
            /* 对于a=b:
             * a在寄存器，b在寄存器：mv a,b
             * a在寄存器，b在内存：lw a,b
             * a在内存，b在寄存器：sw b,a
             * a在内存，b在内存：lw reg,b  sw reg,a
             */
            string addr1 = symbol_to_addr(code.num1);
            if (in_reg(addr1)) {
                load_value(code.num2, addr1);
            } else {
                // a在内存中
                string addr2 = symbol_to_addr(code.num2);
                if (in_reg(addr2)) {
                    save_value(code.num2, addr1);
                } else {
                    // a,b都在内存
                    string reg = "$k0";
                    load_value(code.num2, reg);
                    save_value(reg, code.num1);
                }
            }
        } else if (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV) {
            string addr1 = symbol_to_addr(code.num1);
//            bool is_2_pow_1 = addr1[0] == 'i' && is_2_power(stoi(code.num1));
            if (!in_reg(addr1)) {   //int, char, const, memory
                addr1 = "$k0";
                load_value(code.num1, addr1);
            }
            string addr2 = symbol_to_addr(code.num2);
//            bool is_2_pow_2 = addr2[0] == 'i' && is_2_power(stoi(code.num2));
            if (!in_reg(addr2)) {
                addr2 = "$k1";
                load_value(code.num2, addr2);
            }

            string instr = op_to_instr.find(op)->second;
            string addr3 = symbol_to_addr(code.result);

//            //TODO: consider addr3 in memory
//            if (op == OP_MUL && is_2_pow_1) {
//                generate( "sll " + addr3 + ", " + to_string(int(log2(stoi(code.num1)))) + ", " + addr2);
//            } else if (op == OP_MUL && is_2_pow_2) {
//                generate( "sll " + addr3 + ", " + addr1 + ", " + to_string(int(log2(stoi(code.num2)))));
//            } else if (op == OP_DIV && is_2_pow_1) {
//                generate( "srl " + addr3 + ", " + to_string(int(log2(stoi(code.num1)))) + ", " + addr2);
//            } else if (op == OP_DIV && is_2_pow_2) {
//                generate( "srl " + addr3 + ", " + addr1 + ", " + to_string(int(log2(stoi(code.num2)))));
//            } else {
            if (addr3[0] == '$') {
                generate(instr + " " + addr3 + ", " + addr1 + ", " + addr2);
            } else {
                string reg = "$k0";
                generate(instr + " " + reg + ", " + addr1 + ", " + addr2);
                generate("sw " + reg + ", " + addr3);
            }
        }
    }
}

//将symbol的值读到对应寄存器
void MipsGenerator::load_value(const string &symbol, const string &reg) {
    string addr = symbol_to_addr(symbol);
    switch (addr[0]) {
        case '$':
            generate("mv " + reg + ", " + addr);
            break;
        case 'i':
            generate("li " + reg + ", " + symbol);
            break;
        case 'c':
            generate("li " + reg + ", " + to_string(symbol[1]));
            break;
        case 'C':
            generate("li " + reg + ", " + addr.substr(1, addr.size() - 1));
            break;
        default:
            generate("lw " + reg + ", " + addr);
    }
}

//将reg的值存到symbol的位置
void MipsGenerator::save_value(const string &reg, const string& symbol) {
    string addr = symbol_to_addr(symbol);
    if (addr[0] == '$') {
        generate("mv " + addr + ", " + reg);
    } else {
        generate("sw " + reg + ", " + addr);
    }
}

bool in_memory(string ret) {
    return ret[0] != '$' && ret[0] != 'i' && ret[0] != 'c' && ret[0] != 'C';
}

bool in_reg(string ret) {
    return ret[0] == '$';
}

//返回symbol对应的寄存器或地址，或常量值类型，或常量
string MipsGenerator::symbol_to_addr(const string &symbol) {
    for (int i = 0; i < 10; i++) {
        if (t_reg_table[i] == symbol) {
            return "$t" + to_string(i);
        }
    }
    for (int i = 0; i < 8; i++) {
        if (s_reg_table[i] == symbol) {
            return "$s" + to_string(i);
        }
    }
    if (begins_num(symbol)) {
        return "i"; //int
    }
    if (symbol[0] == '\'') {
        return "c"; //char
    }
    SymTableItem item = SymTable::search(cur_func, symbol);
    if (item.stiType == constant) {
        return "C" + item.const_value; //Const
    }
    SymTableItem global = SymTable::try_search(GLOBAL, symbol, true);
//    if (global.valid && global.stiType == var) {
//        return to_string(item.addr) + "($gp)";
//    }
    //TODO: 超过gp大小时数据存放？
    return to_string(item.addr) + "($sp)";
}

string MipsGenerator::assign_t_reg(const string &name) {
    for (int i = 0; i < 10; i++) {
        if (t_reg_table[i] == VACANT) {
            t_reg_table[i] = name;
            return "$t" + to_string(i);
        }
    }
    return INVALID;
}

string MipsGenerator::assign_s_reg(const string &name) {
    for (int i = 0; i < 8; i++) {
        if (s_reg_table[i] == VACANT) {
            s_reg_table[i] = name;
            return "$s" + to_string(i);
        }
    }
    return INVALID;
}




#pragma clang diagnostic pop