//
// Created by wzk on 2020/10/1.
//

#include "SymTable.h"

#include <utility>

vector<SymTableItem> SymTable::global;
map<string, vector<SymTableItem>> SymTable::local;
unsigned int SymTable::max_name_length = 5;

void SymTable::add(const string &func, const string &name, STIType stiType, DataType dataType, int addr) {
    if (try_search(func, name).valid) {
        Errors::add("redefined identifier '" + name + "'", ERR_REDEFINED);
        return;
    }
    SymTableItem a(lower(name), stiType, dataType, addr);
    a.size = dataType == integer ? 4 : 1;
    if (func == GLOBAL) {
        global.push_back(a);
    } else {
        local[func].push_back(a);
    }
    max_name_length = max_name_length > name.length() ? max_name_length : name.length();
}

void SymTable::add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr) {
    add(func, tk, stiType, dataType, addr, 0, 0);
}

void SymTable::add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr, int dim1, int dim2) {
    if (try_search(func, tk.str).valid) {
        Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
        return;
    }
    SymTableItem a(lower(tk.str), stiType, dataType, addr);
    if (dim1 == 0) {
        a.dim = 0;
    } else if (dim2 == 0) {
        a.dim = 1;
        a.size = size_of(dataType) * dim1;
    } else {
        a.dim = 2;
        a.size = size_of(dataType) * dim1 * dim2;
    }
    if (func == GLOBAL) {
        global.push_back(a);
    } else {
        local[func].push_back(a);
    }
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
}

void SymTable::add_const(const string &func, const Token &tk, DataType dataType, string const_value) {
    if (try_search(func, tk.str).valid) {
        Errors::add("redefined const '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
        return;
    }
    SymTableItem a(lower(tk.str), constant, dataType, 0);
    a.const_value = std::move(const_value);
    if (func == GLOBAL) {
        global.push_back(a);
    } else {
        local[func].push_back(a);
    }
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
}

int SymTable::add_func(const Token &tk, DataType dataType, vector<DataType> types) {
    if (try_search(GLOBAL, tk.str).valid) {
        Errors::add("redefined function '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
        return -1;
    }

    local.insert(make_pair(tk.str, vector<SymTableItem>()));

    SymTableItem a(lower(tk.str), func, dataType, 0);
    a.types = std::move(types);
    global.push_back(a);
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
    return int(global.size() - 1);
}

SymTableItem SymTable::search(const string &func, const Token &tk) {
    if (func != GLOBAL) {
        if (!search_func(func)) {
            return SymTableItem(false);
        }
        for (auto &item: local[func]) {
            if (lower(item.name) == lower(tk.str)) {
                return item;
            }
        }
    }
    for (auto &item: global) {
        if (lower(item.name) == lower(tk.str)) {
            return item;
        }
    }
//        Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, E_UNDEFINED_IDENTF);
    Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_UNDEFINED);
    return SymTableItem(false);
}

SymTableItem SymTable::search(const string &func, const string &str) {
    if (str == "4") {

    }
    if (func != GLOBAL) {
        if (!search_func(func)) {
            return SymTableItem(false);
        }
        for (auto &item: local[func]) {
            if (lower(item.name) == lower(str)) {
                return item;
            }
        }
    }
    for (auto &item: global) {
        if (lower(item.name) == lower(str)) {
            return item;
        }
    }
    Errors::add("undefined identifier '" + str + "'", ERR_UNDEFINED);
    return SymTableItem(false);
}

SymTableItem SymTable::try_search(const string &func, const string &str) {
    if (func != GLOBAL) {
        if (!search_func(func)) {
            return SymTableItem(false);
        }
        for (auto &item: local[func]) {
            if (lower(item.name) == lower(str)) {
                return item;
            }
        }
    }
    for (auto &item: global) {
        if (lower(item.name) == lower(str)) {
            return item;
        }
    }
    return SymTableItem(false);
}

bool SymTable::search_func(const string &func_name) {
    for (auto &item: global) {
        if (lower(item.name) == lower(func_name) && item.stiType == func) {
            return true;
        }
    }
    Errors::add("undefined function '" + func_name + "'", ERR_UNDEFINED);
    return false;
}

void SymTable::show() {
    string sep1, sep2;
    for (int i = 0; i < max_name_length + 20; i++) {
        sep1 += "=";
        sep2 += "-";
    }
    cout << sep1 << endl << "NAME";
    for (int i = 0; i < max_name_length - 3; i++) {
        cout << " ";
    }
    cout << "KIND  TYPE DIM ADDR" << endl << sep2 << endl;
    cout << "---" << GLOBAL << endl;
    for (auto &item: global) {
        cout << item.to_str() << endl;
    }
    for (auto &f: local) {
        cout << sep2 << endl;
        cout << "---" << f.first << endl;
        for (auto &item: f.second) {
            cout << item.to_str() << endl;
        }
    }
    cout << sep1 << endl;
}

string SymTableItem::to_str() const {
    map<STIType, string> stitype_str = {
            {constant, "const"},
            {var,      "var  "},
            {tmp,      "tmp  "},
            {para,     "para "},
            {func,     "func "}
    };

    map<DataType, string> datatype_str = {
            {integer,   "int "},
            {character, "char"},
            {void_ret,  "void"}
    };
    string ans = name;
    for (int i = 0; i < SymTable::max_name_length - name.length(); i++) {
        ans += " ";
    }
    return ans + " " + stitype_str[stiType] + " " + datatype_str[dataType] + " " + to_string(dim) + "   " +
           to_string(addr);
}
