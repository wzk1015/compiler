#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-inefficient-string-concatenation"
//
// Created by wzk on 2020/11/5.
//

#include "MipsGenerator.h"

#include <utility>

string regs[32] = {
        "$zero", "$at", "$v0", "$v1",
        "$a0", "$a1", "$a2", "$a3",
        "$t0", "$t1", "$t2", "$t3",
        "$t4", "$t5", "$t6", "$t7",
        "$s0", "$s1", "$s2", "$s3",
        "$s4", "$s5", "$s6", "$s7",
        "$t8", "$t9", "$k0", "$k1",
        "$gp", "$sp", "$fp", "$ra"
};

map<string, string> op_to_instr = {
        {OP_ADD, "addu"},
        {OP_SUB, "subu"},
        {OP_MUL, "mulu"},
        {OP_DIV, "divu"},
};

vector<string> t_reg_table = {
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT
};

vector<string> s_reg_table = {
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT,
        VACANT
};

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
    generate("lui $gp, 0x1001");
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
            generate(code.num2 + ":");
            generate("addi $sp, $sp, -" + to_string()); //TODO
        }
        else if (op == OP_PRINT) {
            if (code.num2 == "strcon") {
                generate("la $a0, str" + code.num1);
                generate("li $v0, 4");
                generate("syscall");
            } else {
                SymTableItem it = SymTable::search(code.num1); //TODO:考虑临时变量#T1
                load_value(it.name, "$a0");
                if (it.dataType == integer) {
                    generate("li $v0, 1");
                } else {
                    generate("li $v0, 11");
                }
                generate("syscall");
            }
            generate("la $a0, newline");
            generate("li $v0, 4");
            generate("syscall");
        }

        else if (op == OP_SCANF) {
            SymTableItem it = SymTable::search(code.num1); //TODO:考虑临时变量#T1
            if (it.dataType == integer) {
                generate("li $v0, 5");
            } else {
                generate("li $v0, 12");
            }
            generate("syscall");
            save_value("$v0", it.name);
        }

        else if (op == OP_ASSIGN) {
            string addr1 = symbol_to_addr(code.num1);
            if (addr1[0] == '$') {
                load_value(code.num2, addr1);
            } else {
                // num1在内存中
                string addr2 = symbol_to_addr(code.num2);
                if (addr2[0] == '$') {
                    save_value(addr2, addr1);
                } else {
                    //num1,2都在内存
                    string reg = "$k0";
                    load_value(addr2, reg);
                    save_value(reg, addr1);
                }
            }
        }

        else if (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV) {
            string addr1 = symbol_to_addr(code.num1);
            if (addr1[0] != '$') {
                addr1 = "$k0";
                load_value(code.num1, addr1);
            }
            string addr2 = symbol_to_addr(code.num2);
            if (addr2[0] != '$') {
                addr2 = "$k1";
                load_value(code.num2, addr2);
            }

            string instr = op_to_instr.find(op)->second;
            string addr3 = symbol_to_addr(code.result);
            if (addr3[0] == '$') {
                generate(instr + " " + addr3 + ", " + addr1 + ", " + addr2);
            }
            else {
                string reg = "$k0";
                generate(instr + " " + reg + ", " + addr1 + ", " + addr2);
                generate("sw " + reg + ", " + addr3);
            }
            //TODO：优化除法为移位；考虑常数
        }
    }
}

//将symbol的值读到对应寄存器
void MipsGenerator::load_value(const string& symbol, const string &reg) {
    string addr = symbol_to_addr(symbol);
    if (addr[0] == '$') {
        generate("mv " + reg + ", " + addr);
    } else {
        generate("lw " + reg + ", " + addr);
    }
}

//将reg的值存到symbol的位置
void MipsGenerator::save_value(const string &reg, string symbol) {
    string addr = symbol_to_addr(std::move(symbol));
    if (addr[0] == '$') {
        generate("mv " + addr + ", " + reg);
    } else {
        generate("sw " + reg + ", " + addr);
    }
}

//返回symbol对应的寄存器或地址
string MipsGenerator::symbol_to_addr(string symbol) {
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
    return to_string(SymTable::search(symbol).addr)+"($sp)";
}

string MipsGenerator::assign_t_reg(const string& name) {
    for (int i = 0; i < 10; i++) {
        if (t_reg_table[i] == VACANT) {
            t_reg_table[i] = name;
            return "$t" + to_string(i);
        }
    }
    return INVALID;
}

string MipsGenerator::assign_s_reg(const string& name) {
    for (int i = 0; i < 8; i++) {
        if (s_reg_table[i] == VACANT) {
            s_reg_table[i] = name;
            return "$s" + to_string(i);
        }
    }
    return INVALID;
}

void MipsGenerator::show() {
    cout << "=============Mips code=============" << endl;
    for (auto &code: mips) {
        cout << code << endl;
    }
}


#pragma clang diagnostic pop