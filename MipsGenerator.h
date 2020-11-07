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

    explicit MipsGenerator(vector<MidCode> codes, vector<string> strcons) :
            mid(std::move(codes)), strcons(std::move(strcons)) {};

    void generate(const string &code);

    void translate();

    void show();

    void load_value(const string &symbol, const string &reg);

    void save_value(const string &reg, string symbol);

    string symbol_to_addr(string);

    string assign_t_reg(const string&);

    string assign_s_reg(const string&);

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
