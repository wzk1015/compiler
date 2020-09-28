//
// Created by wzk on 2020/9/22.
//

#include "Lexer.h"
#include <map>
#include <sstream>
#include "error.h"


string lower(string wd) {
    string s;
    int len = wd.size();
    for (int i = 0; i < len; i++) {
        if (wd[i] >= 'A' && wd[i] <= 'Z') {
            s += (char) (wd[i] + 'a' - 'A');
        } else {
            s += wd[i];
        }
    }
    return s;
}


int Lexer::get_token() {
    token.clear();
    read_char();
    while (isspace(ch) && pos < source.length()) {
        read_char();
    }

    if (isspace(ch)) {
        //last character of file
        return 1;
    }

    if (isalpha(ch) || ch == '_') {
        while (isalnum(ch) || ch == '_') {
            token += ch;
            read_char();
        }
        retract();
        string reserver_value = reserved(token);
        symbol = (reserver_value == NOTFOUND) ? "IDENFR" : reserver_value;
    } else if (isdigit(ch)) {
        while (isdigit(ch)) {
            token += ch;
            read_char();
        }
        retract();
        symbol = "INTCON";
    } else if (ch == '\'') {
        read_char();
        token = ch;
        symbol = "CHARCON";
        read_char();
        if (ch != '\'') {
            string err = isspace(ch) ? " " : string(&ch);
            throw Error("expected single quote sign ' , got " + err + " instead", line_num, col_num);
        }
    } else if (ch == '\"') {
        while (true) {
            try {
                read_char();
            } catch (Error) {
                throw Error("expected double quote sign \", but reach end of file", line_num, col_num);
            }
            if (ch == '\"') {
                break;
            }
            token += ch;
        }
        symbol = "STRCON";
    } else if (special(ch) != NOTFOUND) {
        token = ch;
        symbol = special(ch);
    } else if (ch == '!') {
        read_char();
        if (ch != '=') {
            string err = isspace(ch) ? " " : string(&ch);
            throw Error("expected '!=', got !" + err + " instead", line_num, col_num);
        }
        token = "!=";
        symbol = "NEQ";
    } else if (ch == '=') {
        read_char();
        if (ch == '=') {
            token = "==";
            symbol = "EQL";
        } else {
            token = "=";
            symbol = "ASSIGN";
            retract();
        }
    } else if (ch == '<') {
        read_char();
        if (ch == '=') {
            token = "<=";
            symbol = "LEQ";
        } else {
            token = "<";
            symbol = "LSS";
            retract();
        }
    } else if (ch == '>') {
        read_char();
        if (ch == '=') {
            token = ">=";
            symbol = "GEQ";
        } else {
            token = ">";
            symbol = "GRE";
            retract();
        }
    } else {
        throw Error("unknown character: " + string(&ch), line_num, col_num);
    }
    if (symbol == "INTCON") {
        int_v = (int) strtol(token.c_str(), nullptr, 10);
    }
    return 0;
}

int Lexer::analyze(const char *in_path, const char *out_path) {
    ifstream in(in_path);
    stringstream buffer;
    buffer << in.rdbuf();
    source = buffer.str();
    in.close();
    if (save_to_file) {
        ofstream out(out_path);
        while (pos < source.length()) {
            if (get_token() == 0) {
                out << symbol << " " << token << endl;
                num_tokens++;
            }
        }
        out.close();
    } else {
        while (pos < source.length())
            get_token();
    }

    cout << "Lexer complete successfully. Extracted " << num_tokens << " tokens." << endl;
    return 0;
}

int Lexer::read_char() {
    if (pos >= source.length()) {
        throw Error("unexpected end of file", line_num, col_num);
    }
    ch = source[pos];
    pos++;
    if (ch == '\n') {
        line_num++;
        col_num = 0;
    } else {
        col_num++;
    }
    return 0;
}

void Lexer::retract() {
    pos--;
    col_num--;
}

string Lexer::reserved(string tk) {
    map<string, string> reserves = {
            {"const",   "CONSTTK"},
            {"int",     "INTTK"},
            {"char",    "CHARTK"},
            {"void",    "VOIDTK"},
            {"main",    "MAINTK"},
            {"if",      "IFTK"},
            {"else",    "ELSETK"},
            {"switch",  "SWITCHTK"},
            {"case",    "CASETK"},
            {"default", "DEFAULTTK"},
            {"while",   "WHILETK"},
            {"for",     "FORTK"},
            {"scanf",   "SCANFTK"},
            {"printf",  "PRINTFTK"},
            {"return",  "RETURNTK"},
    };
    auto iter = reserves.find(lower(std::move(tk)));
    if (iter != reserves.end()) {
        return iter->second;
    }
    return NOTFOUND;
}

string Lexer::special(char tk) {
    map<char, string> specials = {
            {'+', "PLUS"},
            {'-', "MINU"},
            {'*', "MULT"},
            {'/', "DIV"},
            {':', "COLON"},
            {';', "SEMICN"},
            {',', "COMMA"},
            {'(', "LPARENT"},
            {')', "RPARENT"},
            {'[', "LBRACK"},
            {']', "RBRACK"},
            {'{', "LBRACE"},
            {'}', "RBRACE"}
    };
    auto iter = specials.find(tk);
    if (iter != specials.end()) {
        return iter->second;
    }
    return NOTFOUND;
}


