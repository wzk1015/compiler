//
// Created by wzk on 2020/10/1.
//

#include "SymTable.h"

vector<SymTableItem> SymTable::items;
vector<int> SymTable::layers;
unsigned int SymTable::max_name_length = 5;

void SymTable::add(const string &name, STIType stiType, DataType dataType, int addr) {
    int start = layers.empty() ? 0 : layers[layers.size() - 1];
    for (int i = start; i < items.size(); i++) {
        if (items[i].name == name) {
//                    Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, E_REDEFINED_IDENTF);
            Errors::add("redefined identifier '" + name + "'", ERR_REDEFINED);
            return;
        }
    }
    SymTableItem a(lower(name), stiType, dataType, addr);
    a.size = dataType == integer ? 4 : 1;
    items.push_back(a);
    max_name_length = max_name_length > name.length() ? max_name_length : name.length();
}

void SymTable::add(const Token &tk, STIType stiType, DataType dataType, int addr) {
    add(tk, stiType, dataType, addr, 0, 0);
}

void SymTable::add(const Token &tk, STIType stiType, DataType dataType, int addr, int dim1, int dim2) {
    int start = layers.empty() ? 0 : layers[layers.size() - 1];
    for (int i = start; i < items.size(); i++) {
        if (items[i].name == tk.str) {
//                    Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, E_REDEFINED_IDENTF);
            Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
            return;
        }
    }
    SymTableItem a(lower(tk.str), stiType, dataType, addr);
    if (dim1 == 0) {
        a.dim = 0;
    } else if (dim2 == 0) {
        a.dim = 1;
        a.size = (dataType == integer ? 4 : 1) * dim1;
    } else {
        a.dim = 2;
        a.size = (dataType == integer ? 4 : 1) * dim1 * dim2;
    }
    items.push_back(a);
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
}

int SymTable::add_func(const Token &tk, DataType dataType, vector<DataType> types) {
    int start = layers.empty() ? 0 : layers[layers.size() - 1];
    for (int i = start; i < items.size(); i++) {
        if (items[i].name == tk.str) {
//                    Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, E_REDEFINED_IDENTF);
            Errors::add("redefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_REDEFINED);
            return i;
        }
    }
    SymTableItem a(lower(tk.str), func, dataType, 0);
    a.types = std::move(types);
    items.push_back(a);
    max_name_length = max_name_length > tk.str.length() ? max_name_length : tk.str.length();
    return int(items.size() - 1);
}

void SymTable::add_layer() {
//        if (DEBUG) {
//            cout << "add layer. layers: " << layers.size() << endl;
//            show();
//        }
    layers.push_back(items.size());
}

void SymTable::pop_layer() {
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

SymTableItem SymTable::search(const Token &tk) {
    string str = lower(tk.str);
    for (int i = items.size() - 1; i >= 0; i--) {
        if (items[i].name == str) {
            return items[i];
        }
    }
//        Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, E_UNDEFINED_IDENTF);
    Errors::add("undefined identifier '" + tk.str + "'", tk.line, tk.column, ERR_UNDEFINED);
    return SymTableItem(false);
}

SymTableItem SymTable::search(const string &str) {
    for (int i = items.size() - 1; i >= 0; i--) {
        if (items[i].name == str) {
            return items[i];
        }
    }
    Errors::add("undefined identifier '" + str + "'", ERR_UNDEFINED);
    return SymTableItem(false);
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
    return ans + " " + stitype_str[stiType] + " " + datatype_str[dataType] + " " + to_string(dim) + " " +
           to_string(addr);
}
