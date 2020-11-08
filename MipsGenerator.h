//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_MIPSGENERATOR_H
#define COMPILER_MIPSGENERATOR_H

#include <utility>

#include "MidCode.h"
#include "Error.h"
#include "SymTable.h"

class MipsGenerator {
public:
    vector<MidCode> mid;
    vector<string> mips;
    vector<string> strcons;
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
    string cur_func = GLOBAL;
    int sp_size = 0;

    explicit MipsGenerator(vector<MidCode> codes, vector<string> strcons) :
            mid(std::move(codes)), strcons(std::move(strcons)) {};

    void generate(const string &code);

    void translate();

    void show();

    void load_value(const string &symbol, const string &reg);

    void save_value(const string &reg, string symbol);

    string symbol_to_addr(const string &);

    string assign_t_reg(const string &);

    string assign_s_reg(const string &);

//    string allocate_memory();
};

//const string op[] = {
//    ".data", ".text",
//    ".space", ".asciiz",
//    ":",
//    "syscall",
//    "addu", "subu", "addiu", "subiu",
//    "mult", "mul", "div",
//    "sll", "srl",
//    "move", "lui",
//    "li", "la",
//    "lw", "sw",
//    "mfhi", "mflo",
//    "j", "jr", "jal",
//    "beq", "bne", "blt", "ble", "bgt", "bge"
//};




#endif //COMPILER_MIPSGENERATOR_H
