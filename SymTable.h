//
// Created by wzk on 2020/10/1.
//

#ifndef COMPILER_SYMTABLE_H
#define COMPILER_SYMTABLE_H

#include <iostream>
#include <map>
#include <utility>
#include "Error.h"
#include "Token.h"

#define GLOBAL "global"
#define size_of(dt)  4
//(dt == integer ? 4 : 1)

using namespace std;

enum STIType {
    constant,
    var,
    tmp,
    para,
    func
};

enum DataType {
    integer,
    character,
    void_ret,
    invalid
};

class SymTableItem {
public:
    string name;
    STIType stiType{};
    DataType dataType{};
    int dim = 0;
    bool valid = true;
    vector<DataType> types;
    int addr{};
    int size{};

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

    static void add(const string &func, const string &name, STIType stiType, DataType dataType, int addr);

    static void add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr);

    static void add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr, int dim1, int dim2);

    static int add_func(const Token &tk, DataType dataType, vector<DataType> types);

    static SymTableItem search(const string &func, const Token &tk);

    static SymTableItem search(const string &func, const string &str);

    static SymTableItem try_search(const string &func, const string &str);

    static bool search_func(const string &func_name);

    static void show();
};


#endif //COMPILER_SYMTABLE_H
