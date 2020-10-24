//
// Created by wzk on 2020/9/22.
//

#include "Lexer.h"


Token Lexer::get_token() {
    token.clear();
    read_char();
    while (isspace(ch) && pos < source.length()) {
        read_char();
    }

    if (isspace(ch)) {
        //last character of file
        return Token(INVALID);
    }

    Token r(INVALID, INVALID, line_num, col_num, pos);

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
//            Errors::add("expected single quote sign ' , got " + err + " instead", line_num, col_num,
//                                E_UNEXPECTED_CHAR);
            Errors::add("expected single quote sign ' , got " + err + " instead", line_num, col_num,
                                ERR_LEXER);
        }
    } else if (ch == '\"') {
        while (true) {
            read_char();
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
//            Errors::add("expected '!=', got !" + err + " instead", line_num, col_num, E_UNEXPECTED_CHAR);
            Errors::add("expected '!=', got !" + err + " instead", line_num, col_num, ERR_LEXER);
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
//        Errors::add("unknown character: " + string(&ch), line_num, col_num, E_UNKNOWN_CHAR);
        Errors::add("unknown character: " + string(&ch), line_num, col_num, ERR_LEXER);
    }

    r.type = symbol;
    r.str = token;
    if (symbol == "INTCON") {
        r.v_int = (int) strtol(token.c_str(), nullptr, 10);
    }
    else if (symbol == "STRCON") {
        if (token.empty()) {
//            Errors::add("empty string", line_num, col_num, E_UNEXPECTED_CHAR);
            Errors::add("empty string", line_num, col_num, ERR_LEXER);
        }
        for (auto &c: token) {
            if (c <= 31 || c == 34 || c >= 127) {
//                Errors::add("invalid ascii character in string: " + string(&c),
//                                    line_num, col_num, E_UNKNOWN_CHAR);
                Errors::add("invalid ascii character in string: " + string(&c),
                                    line_num, col_num, ERR_LEXER);
            }
        }
    }
    else if (symbol == "CHARCON") {
        r.v_char = token.c_str()[0];
        if (r.v_char != '+' && r.v_char != '-' && r.v_char != '*'
            && r.v_char != '/' && r.v_char != '_' && !isalnum(r.v_char)) {
//            Errors::add("invalid character: " + string(&r.v_char),
//                                line_num, col_num, E_UNKNOWN_CHAR);
            Errors::add("invalid character: " + string(&r.v_char),
                                line_num, col_num, ERR_LEXER);
        }
    }
    return r;
}

Token Lexer::analyze() {
    try {
        return get_token();
    } catch (exception ex) {
//        Errors::add("unexpected end of file", E_UNEXPECTED_EOF);
        Errors::add("unexpected end of file", ERR_LEXER);
    }
    return Token(INVALID);
}

int Lexer::read_char() {
    if (pos >= source.length()) {
        throw exception();
    }
    ch = source[pos++];
    if (ch == '\n') {
        line_num++;
        col_num = 1;
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


