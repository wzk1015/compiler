//
// Created by wzk on 2020/9/22.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include "LexResults.h"
#include "Error.h"

using namespace std;

class Lexer {
public:
    char ch{};
    string token;
    string symbol;
    string source;
    int pos = 0;
    int line_num = 1;
    int col_num = 1;


    LexResults analyze();
    LexResults get_token();
    int read_char();
    void retract();
    static string special(char);
    static string reserved(string);
    explicit Lexer(const string& in_path) {
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