//
// Created by wzk on 2020/10/1.
//

#ifndef COMPILER_SYMTABLE_H
#define COMPILER_SYMTABLE_H

#include <iostream>
#include <map>
#include <utility>
#include "Error.h"
#include "Lexer.h"

#define GLOBAL "#GLOBAL"
#define size_of(dt)  (4)
//(dt == integer ? 4 : 1)
#define type_to_str(dt) (dt == integer? "int" : "char")

#define LOCAL_ADDR_INIT 100

using namespace std;

enum STIType {
    invalid_sti,
    constant,
    var,
    tmp,
    para,
    func
};

enum DataType {
    invalid_dt,
    integer,
    character,
    void_ret

};

class SymTableItem {
public:
    string name;
    STIType stiType{};
    DataType dataType{};
    int dim = 0;
    bool valid = true;
    vector<pair<DataType, string>> paras;
    int addr{};
    int size{};
    int dim1_size{};
    int dim2_size{};
    string const_value;
    bool modified = false;
    bool recur_func = false;


    SymTableItem(string name, STIType stiType1, DataType dataType1, int addr) :
            name(std::move(name)), stiType(stiType1), dataType(dataType1), addr(addr) {};

    explicit SymTableItem(bool valid) : valid(valid) {};

    SymTableItem() = default;

    string to_str() const;
};

class SymTable {
public:
    static vector<SymTableItem> global;
    static map<string, vector<SymTableItem>> local;
    static unsigned int max_name_length;
    static SymTableItem invalid;

    static void add(const string &func, const string &name, STIType stiType, DataType dataType, int addr);

    static void add(const string &func, const string &name, STIType stiType, DataType dataType, int addr, int dim1, int dim2);

    static void add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr);

    static void add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr, int dim1, int dim2);

    static void add_const(const string &func, const Token &tk, DataType dataType, string const_value);

    static int add_func(const Token &tk, DataType dataType, vector<pair<DataType, string>> paras);

    static SymTableItem search(const string &func, const Token &tk);

    static SymTableItem search(const string &func, const string &str);

    static SymTableItem try_search(const string &func, const string &str, bool include_global);

    static bool in_global(const string &func, const string &str);

    static SymTableItem search_func(const string &func_name);

    static void show();

    static SymTableItem &ref_search(const string &func, const string &str);

    static int func_size(const string &func);

    static void reset();
};


#endif //COMPILER_SYMTABLE_H
