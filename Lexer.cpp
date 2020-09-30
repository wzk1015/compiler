//
// Created by wzk on 2020/9/22.
//

#include "Lexer.h"
#include <map>
#include <vector>
#include <sstream>
#include "error.h"
#include "LexResults.h"

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


LexResults Lexer::get_token() {
    token.clear();
    read_char();
    while (isspace(ch) && pos < source.length()) {
        read_char();
    }

    if (isspace(ch)) {
        //last character of file
        return LexResults(INVALID, INVALID, -1, -1, -1);
    }

    LexResults r(INVALID, INVALID, line_num, col_num, pos);

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
            errors.emplace_back("expected single quote sign ' , got " + err + " instead", line_num, col_num,
                                E_UNEXPECTED_CHAR);
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
            errors.emplace_back("expected '!=', got !" + err + " instead", line_num, col_num, E_UNEXPECTED_CHAR);
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
        errors.emplace_back("unknown character: " + string(&ch), line_num, col_num, E_UNKNOWN_CHAR);
    }

    r.type = symbol;
    r.str = token;
    if (symbol == "INTCON") {
        r.v_int = (int) strtol(token.c_str(), nullptr, 10);
    }
    else if (symbol == "STRCON") {
        if (token.empty()) {
            errors.emplace_back("empty string", line_num, col_num, E_UNEXPECTED_CHAR);
        }
        for (auto &c: token) {
            if (c <= 31 || c == 34 || c >= 127) {
                errors.emplace_back("invalid ascii character in string: " + string(&c),
                                    line_num, col_num, E_UNKNOWN_CHAR);
            }
        }
    }
    else if (symbol == "CHARCON") {
        r.v_char = token.c_str()[0];
        if (r.v_char != '+' && r.v_char != '-' && r.v_char != '*'
            && r.v_char != '/' && r.v_char != '_' && !isalnum(r.v_char)) {
            errors.emplace_back("invalid character: " + string(&r.v_char),
                                line_num, col_num, E_UNKNOWN_CHAR);
        }
    }
    return r;
}

vector<LexResults> Lexer::analyze(const char *in_path, const char *out_path) {
    ifstream in(in_path);
    stringstream buffer;
    buffer << in.rdbuf();
    source = buffer.str();
    in.close();
    ofstream out;

    vector<LexResults> results;

    if (save_to_file) {
        out.open(out_path);
    }
    while (pos < source.length()) {
        try {
            LexResults r = get_token();
            if (r.type != INVALID) {
                results.push_back(r);
                if (save_to_file) {
                    out << symbol << " " << token << endl;
                }
                num_tokens++;
            }
        } catch (exception) {
            errors.emplace_back("unexpected end of file", E_UNEXPECTED_EOF);
            break;
        }
    }
    out.close();

    cout << "Lexer complete successfully. Extracted " << num_tokens << " tokens." << endl;
    return results;
}

int Lexer::read_char() {
    if (pos >= source.length()) {
        throw exception();
    }
    ch = source[pos++];
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


