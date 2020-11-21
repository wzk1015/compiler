//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_MIPSGENERATOR_H
#define COMPILER_MIPSGENERATOR_H

#include <utility>
#include <cmath>

#include "MidCode.h"
#include "Error.h"
#include "SymTable.h"

#define STACK_RA "0($sp)"
#define STACK_V0 "4($sp)" //unused
#define STACK_A_BEGIN 8
#define STACK_S_BEGIN 24
#define STACK_T_BEGIN 56
#define STACK_RESERVED "96($sp)"



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
            {OP_MUL, "mul"},
            {OP_DIV, "div"},
    };
    string cur_func = GLOBAL;
    vector<vector<SymTableItem>> call_func_paras;
    vector<int> sp_size = {0};
    int call_func_sp_offset = 0;
    map<string, int> func_sp_offsets;

    MipsGenerator(): mid(MidCodeList::codes), strcons(MidCodeList::strcons) {};

    void generate(const string &code);

    void generate(const string &op, const string &num1);

    void generate(const string &op, const string &num1, const string& num2);

    void generate(const string &op, const string &num1, const string& num2, const string& num3);

    void translate_assign(const string &num1, const string &num2);

    void translate();

    void show() {
        cout << "=============Mips code=============" << endl;
        for (auto &code: mips) {
            cout << code << endl;
        }
    }

    void save_to_file(const string &out_path) {
        ofstream out(out_path);
        for (auto &c: mips) {
            out << c << endl;
        }
        out.close();
    }

    void load_value(const string &symbol, const string &reg);

    void save_value(const string &reg, const string &symbol);

    string symbol_to_addr(const string &);

    string assign_t_reg(const string &);

    string assign_s_reg(const string &);

    bool assign_reg(const string& symbol);

    bool in_reg(const string& symbol);

    bool in_memory(const string& symbol);

    static bool is_const(const string& symbol) ;

    void show_reg_status();

    void release(string);

//    string allocate_memory();
};

bool in_reg(string);

bool in_memory(string);



#endif //COMPILER_MIPSGENERATOR_H
