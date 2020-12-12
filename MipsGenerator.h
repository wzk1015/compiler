//
// Created by wzk on 2020/11/5.
//

#ifndef COMPILER_MIPSGENERATOR_H
#define COMPILER_MIPSGENERATOR_H

#include <utility>
#include <cmath>

#include "PseudoCode.h"
#include "Error.h"
#include "SymTable.h"

#define STACK_RA "0($sp)"
#define STACK_V0 "4($sp)" //unused
#define STACK_A_BEGIN 8
#define STACK_S_BEGIN 24
#define STACK_T_BEGIN 56
#define STACK_RESERVED "96($sp)"

#define NUM_T_REG 4
#define NUM_S_REG 16


class MipsGenerator {
public:
    vector<PseudoCode> mid;
    vector<string> mips;
    vector<string> strcons;
    vector<string> s_regs = {
            "$s0", "$s1", "$s2", "$s3",
            "$s4", "$s5", "$s6", "$s7",
            "$t4", "$t5", "$t6", "$t7",
            "$t8", "$t9", "$k0", "$k1"
    };

    vector<string> t_regs = {
            "$t0", "$t1", "$t2", "$t3"
    };

    vector<string> s_reg_table = {
            VACANT, VACANT, VACANT, VACANT, VACANT,
            VACANT, VACANT, VACANT, VACANT, VACANT,
            VACANT, VACANT, VACANT, VACANT, VACANT,
            VACANT //, VACANT
    };
    vector<string> t_reg_table = {
            VACANT, VACANT, VACANT, VACANT
    };
    vector<int> s_reg_last_use = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0 //, 0
    };
    int clock = 0;
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
    string fp_content = INVALID;
    int s_assign_begin = 0;


    map<string, string> op_to_instr = {
            {OP_ADD, "addu"},
            {OP_SUB, "subu"},
            {OP_MUL, "mul"},
            {OP_DIV, "div"},
            {OP_SRA, "srav"}
    };
    string cur_func = GLOBAL;
    int call_func_sregs = 0;
    int cur_func_begin = 0;
    vector<vector<SymTableItem>> call_func_paras;
    vector<int> sp_size = {0};
    int call_func_sp_offset = 0;
    int tmp_label_idx = 1;

    bool optimize_assign_reg = false;
    bool optimize_muldiv = false;
    bool optimize_2pow = false;

    bool rel = true;

    MipsGenerator(): mid(PseudoCodeList::codes), strcons(PseudoCodeList::strcons) {};

    void generate(const string &code);

    void generate(const string &op, const string &num1);

    void generate(const string &op, const string &num1, const string& num2);

    void generate(const string &op, const string &num1, const string& num2, const string& num3);

    void gen_assign(const string &num1, const string &num2);

    void translate();

    void gen_arithmetic(const string &op, const string &num1, const string& num2, const string& num3);

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

    bool assign_reg(const string& symbol, bool only_para=false);

    bool in_reg(const string& symbol);

    bool in_memory(const string& symbol);

    static bool is_const(const string& symbol) ;

    void show_reg_status();

    void release(string);

    string assign_label() {
        string ret = "tmp_label_" + to_string(tmp_label_idx);
        tmp_label_idx++;
        return ret;
    }

    int s_used() {
        int ret = 0;
        for (int i = 0; i < NUM_S_REG; i++) {
            if (s_reg_table[i] == VACANT) {
                ret++;
            }
        }
        return ret;
    }

//    string allocate_memory();
};



#endif //COMPILER_MIPSGENERATOR_H
