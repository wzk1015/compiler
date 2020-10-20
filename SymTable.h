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

using namespace std;

enum STIType {
    constant,
    var,
    para,
    func
};

enum DataType {
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

    SymTableItem(string name, STIType stiType1, DataType dataType1) :
            name(std::move(name)), stiType(stiType1), dataType(dataType1) {};

    explicit SymTableItem(bool valid) : valid(valid) {};

    SymTableItem() = default;

    string to_str() const;
};

class SymTable {
public:
    static vector<SymTableItem> items;
    static vector<int> layers;
    static unsigned int max_name_length;

    static void add(const Token& tk, STIType stiType, DataType dataType) {
        add(tk, stiType, dataType, 0);
    }

    static void add(const Token& tk, STIType stiType, DataType dataType, int dim) {
        if (!layers.empty()) {
            for (int i = layers[layers.size() - 1]; i < items.size(); i++) {
                if (items[i].name == tk.str) {
                    Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, E_REDEFINED_IDENTF);
                    return;
                }
            }
        }
        SymTableItem a(tk.str, stiType, dataType);
        a.dim = dim;
        items.push_back(a);
        max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
    }

    static void add_layer() {
        if (DEBUG) {
            cout << "add layer. layers: " << layers.size() << endl;
            show();
        }
        layers.push_back(items.size());
    }

    static void pop_layer() {
        if (DEBUG) {
            cout << "pop layer. layers: " << layers.size() << endl;
            show();
        }
        unsigned int pop_items_count = items.size() - layers[layers.size() - 1];
        for (int i = 0; i < pop_items_count; i++) {
            items.pop_back();
        }
        layers.pop_back();
    }

    static SymTableItem search(const Token &tk) {
        for (unsigned int i = items.size() - 1; i >= 0; i--) {
            if (items[i].name == tk.str) {
                return items[i];
            }
        }
        Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, E_UNDEFINED_IDENTF);
        return SymTableItem(false);
    }

    static void show() {
        string sep1, sep2;
        for (int i = 0; i < max_name_length + 15; i++) {
            sep1 += "=";
            sep2 += "-";
        }
        cout << sep1 << endl << "NAME";
        for (int i = 0; i < max_name_length - 3; i++) {
            cout << " ";
        }
        cout << "KIND  TYPE DIM" << endl << sep2 << endl;
        int l = 0;
        for (int i = 0; i < items.size(); i++) {
            if (!layers.empty() && i == layers[l]) {
                l++;
                cout << sep2 << endl;
            }
            cout << items[i].to_str() << endl;
        }
        cout << sep1 << endl;
    }
};


#endif //COMPILER_SYMTABLE_H
