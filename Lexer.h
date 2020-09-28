//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H
#include <iostream>
#include <fstream>
using namespace std;

class Lexer {
private:
    char ch{};
    string token;
    string symbol;
    string source;
    int pos = 0;
    int line_num = 1;
    int col_num = 1;
    bool save_to_file = false;
    int int_v = -1;
    int num_tokens = 0;

public:
    int read_char();
    int analyze(const char *, const char *);
    int get_token();
    void retract();
    static string special(char);
    static string reserved(string);
    explicit Lexer(bool save_to_file): save_to_file(save_to_file) {};
};


#endif
