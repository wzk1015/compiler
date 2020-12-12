//
// Created by wzk on 2020/10/1.
//

#include "SymTable.h"

#include <utility>

vector<SymTableItem> SymTable::global;
map<string, vector<SymTableItem>> SymTable::local;
unsigned int SymTable::max_name_length = 5;
SymTableItem SymTable::invalid = SymTableItem(false);


void SymTable::add(const string &func, const string &name, STIType stiType, DataType dataType, int addr) {
    if (try_search(func, name, false).valid) {
        Errors::add("redefined identifier '" + name + "'", ERR_REDEFINED);
        return;
    }
    SymTableItem a(name, stiType, dataType, addr);
    a.size = size_of(dataType);
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

void
SymTable::add(const string &func, const Token &tk, STIType stiType, DataType dataType, int addr, int dim1, int dim2) {
    if (try_search(func, tk.str, false).valid) {
        Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
        return;
    }
    SymTableItem a(tk.str, stiType, dataType, addr);
    if (dim1 == 0) {
        a.dim = 0;
    } else if (dim2 == 0) {
        a.dim = 1;
        a.dim1_size = dim1;
        a.size = size_of(dataType) * dim1;
//        a.arr_value = vector<string>(dim1);
    } else {
        a.dim = 2;
        a.dim1_size = dim1;
        a.dim2_size = dim2;
        a.size = size_of(dataType) * dim1 * dim2;
//        a.arr_value = vector<string>(dim1 * dim2);
    }
    if (func == GLOBAL) {
        global.push_back(a);
    } else {
        local[func].push_back(a);
    }
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
}

void
SymTable::add(const string &func, const string &name, STIType stiType, DataType dataType, int addr, int dim1, int dim2) {
    if (try_search(func, name, false).valid) {
        Errors::add("redefined identifier '" + name + "'", ERR_REDEFINED);
        return;
    }
    SymTableItem a(name, stiType, dataType, addr);
    if (dim1 == 0) {
        a.dim = 0;
    } else if (dim2 == 0) {
        a.dim = 1;
        a.dim1_size = dim1;
        a.size = size_of(dataType) * dim1;
//        a.arr_value = vector<string>(dim1);
    } else {
        a.dim = 2;
        a.dim1_size = dim1;
        a.dim2_size = dim2;
        a.size = size_of(dataType) * dim1 * dim2;
//        a.arr_value = vector<string>(dim1 * dim2);
    }
    if (func == GLOBAL) {
        global.push_back(a);
    } else {
        local[func].push_back(a);
    }
    max_name_length = max_name_length > name.length() ? max_name_length : name.length();
}

void SymTable::add_const(const string &func, const Token &tk, DataType dataType, string const_value) {
    if (try_search(func, tk.str, false).valid) {
        Errors::add("redefined const '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
        return;
    }
    SymTableItem a(tk.str, constant, dataType, 0);
    a.const_value = std::move(const_value);
    if (func == GLOBAL) {
        global.push_back(a);
    } else {
        local[func].push_back(a);
    }
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
}

int SymTable::add_func(const Token &tk, DataType dataType, vector<pair<DataType, string>> paras) {
    if (try_search(GLOBAL, tk.str, true).valid) {
        Errors::add("redefined function '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
        return -1;
    }

    local[tk.str] = vector<SymTableItem>();
    //local.insert(make_pair(tk.str, vector<SymTableItem>()));

    SymTableItem a(tk.str, func, dataType, 0);
    a.paras = std::move(paras);
    global.push_back(a);
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
    return int(global.size() - 1);
}

SymTableItem SymTable::search(const string &func, const Token &tk) {
    if (func != GLOBAL) {
        if (!search_func(func).valid) {
            return SymTableItem(false);
        }
        vector<SymTableItem> loc = local.find(func)->second;
        for (auto &item: loc) {
            if (item.name == tk.str) {
                return item;
            }
        }
    }
    for (auto &item: global) {
        if (item.name == tk.str) {
            return item;
        }
    }
//        Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, E_UNDEFINED_IDENTF);
    Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_UNDEFINED);
    return SymTableItem(false);
}

SymTableItem SymTable::search(const string &func, const string &str) {
    if (func != GLOBAL) {
        if (!search_func(func).valid) {
            return SymTableItem(false);
        }
        vector<SymTableItem> loc = local.find(func)->second;
        for (auto &item: loc) {
            if (item.name == str) {
                return item;
            }
        }
    }
    for (auto &item: global) {
        if (item.name == str) {
            return item;
        }
    }
    Errors::add("undefined identifier '" + str + "'", ERR_UNDEFINED);
    return SymTableItem(false);
}

SymTableItem &SymTable::ref_search(const string &func, const string &str) {
    if (func != GLOBAL) {
        if (!search_func(func).valid) {
            return invalid;
        }
        for (auto &item: local.find(func)->second) {
            if (item.name == str) {
                return item;
            }
        }
    }
    for (auto &item: global) {
        if (item.name == str) {
            return item;
        }
    }
    //Errors::add("undefined identifier '" + str + "'", ERR_UNDEFINED);
    return invalid;
}

SymTableItem SymTable::try_search(const string &func, const string &str, bool include_global) {
    if (func != GLOBAL) {
        if (!search_func(func).valid) {
            return SymTableItem(false);
        }
        vector<SymTableItem> loc = local.find(func)->second;
        for (auto &item: loc) {
            if (item.name == str) {
                return item;
            }
        }
    }
    if (include_global) {
        for (auto &item: global) {
            if (item.name == str) {
                return item;
            }
        }
    }
    return SymTableItem(false);
}

SymTableItem SymTable::search_func(const string &func_name) {
    for (auto &item: global) {
        if (item.name == func_name && item.stiType == func) {
            return item;
        }
    }
    Errors::add("undefined function '" + func_name + "'", ERR_UNDEFINED);
    return invalid;
}

void SymTable::show() {
    string sep1, sep2;
    for (int i = 0; i < max_name_length + 26; i++) {
        sep1 += "=";
        sep2 += "-";
    }
    cout << sep1 << endl << "NAME";
    for (int i = 0; i < max_name_length - 3; i++) {
        cout << " ";
    }
    cout << "KIND  TYPE DIM ADDR VALUE" << endl << sep2 << endl;
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

bool SymTable::in_global(const string &func, const string &str) {
    return !try_search(func, str, false).valid && try_search(GLOBAL, str, true).valid;
}

int SymTable::func_size(const string &func) {
    vector<SymTableItem> vec = func == GLOBAL ? global : local[func];
    if (!vec.empty()) {
        for (auto it = vec.end() - 1; it >= vec.begin(); it--) {
            if (it->stiType != constant) {
                return it->addr + it->size;
            }
        }
    }
    return 0;
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
    string sep = addr >= 10000 ? "     " : addr >= 1000 ? " " : addr >= 100 ? "  " : addr >= 10 ? "   " : "    ";
    return ans + " " + stitype_str[stiType] + " " + datatype_str[dataType] + " " + to_string(dim) + "   " +
           to_string(addr) + sep + const_value;
}

void SymTable::reset() {
    global.clear();
    local.clear();
    max_name_length = 5;
}

int SymTable::func_var_paras(const string &func) {
    int ret = 0;
    for (auto &item: local[func]) {
        if (item.stiType == var || item.stiType == para) {
            ret++;
        }
    }
    return ret;
}
