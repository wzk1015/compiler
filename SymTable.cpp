//
// Created by wzk on 2020/10/1.
//

#include "SymTable.h"

vector<SymTableItem> SymTable::items;
vector<int> SymTable::layers;
unsigned int SymTable::max_name_length = 5;


string SymTableItem::to_str() const {
    map<STIType, string> stitype_str = {
        {constant, "const"},
        {var, "var  "},
        {para, "para "},
        {func, "func "}
    };

    map<DataType, string> datatype_str = {
        {integer, "int "},
        {character, "char"},
        {void_ret, "void"}
    };
    string ans = name;
    for (int i = 0; i < SymTable::max_name_length - name.length(); i++) {
        ans += " ";
    }
    return ans + " " + stitype_str[stiType] + " " + datatype_str[dataType] + " " + to_string(dim);
}
