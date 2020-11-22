//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <sstream>

#include "Error.h"

using namespace std;

class Token {
public:
    string type;
    string str;
    string original_str;
    int v_int = -1;
    char v_char = 'E';
    int line{};
    int column{};
    int pos{};

    Token(string t, string s, int l, int c, int p) :
            type(std::move(t)), str(std::move(s)), line(l), column(c), pos(p) {};
    explicit Token(string t) : type(std::move(t)) {};
    Token(string t, string s) : type(std::move(t)), str(std::move(s)) {};
};

class Lexer {
public:
    char ch{};
    string token;
    string symbol;
    string source;
    int pos = 0;
    int line_num = 1;
    int col_num = 1;
    bool replace_mode;


    Token analyze();
    Token get_token();
    int read_char();
    void retract();
    static string special(char);
    static string reserved(string);
    explicit Lexer(const string& in_path, bool replace) {
        replace_mode = replace;
        ifstream in(in_path);
        stringstream buffer;
        buffer << in.rdbuf();
        source = buffer.str() + "\n";
        if (source.empty()) {
            Errors::add("file not found or empty", E_EMPTY_FILE);
        }
        in.close();
    };
};


#endif

//标识符   IDENFR	else	ELSETK	    -	MINU	=	ASSIGN
//整形常量 INTCON	    switch	SWITCHTK	*	MULT	;	SEMICN
//字符常量 CHARCON	case	CASETK	    /	DIV	    ,	COMMA
//字符串   STRCON	default	DEFAULTTK	<	LSS	    (	LPARENT
//const	  CONSTTK	while	WHILETK	    <=	LEQ	    )	RPARENT
//int	  INTTK	    for	    FORTK	    >	GRE	    [	LBRACK
//char	  CHARTK	scanf	SCANFTK	    >=	GEQ	    ]	RBRACK
//void	  VOIDTK	printf	PRINTFTK	==	EQL	    {	LBRACE
//main	  MAINTK	return	RETURNTK	!=	NEQ	    }	RBRACE
//if	  IFTK	+	PLUS	：	COLON
