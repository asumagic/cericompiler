%{
// This is our Lexical tokeniser
// It should be compiled into cpp with :
// flex++ -d -otokeniser.cpp tokeniser.l
// And then compiled into object with
// g++ -c tokeniser.cpp
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

#include "token.hpp"
#include <iostream>

using namespace std;

%}

%option noyywrap
%option c++
%option yylineno

stringconst  \"[^\n"]+\"
alpha   [A-Za-z]
digit   [0-9]
charliteral \'[^\n\r\t]\'
floatliteral    {digit}+\.{digit}+
integerliteral  {digit}+
id	{alpha}({alpha}|{digit})*
unknown [^\"A-Za-z0-9 \n\r\t\(\)\<\>\=\!\%\&\|\}\-\;\.]+

%%

"BEGIN"   return KEYWORD_BEGIN;
"END"     return KEYWORD_END;
"WHILE"   return KEYWORD_WHILE;
"FOR"     return KEYWORD_FOR;
"TO"      return KEYWORD_TO;
"DO"      return KEYWORD_DO;
"IF"      return KEYWORD_IF;
"THEN"    return KEYWORD_THEN;
"ELSE"    return KEYWORD_ELSE;
"DISPLAY" return KEYWORD_DISPLAY;
"VAR"     return KEYWORD_VAR;
"TYPE"    return KEYWORD_TYPE;
"CONVERT" return KEYWORD_CONVERT;
"FFI"     return KEYWORD_FFI;

"INTEGER" return TYPE_INTEGER;
"DOUBLE"  return TYPE_DOUBLE;
"BOOLEAN" return TYPE_BOOLEAN;
"CHAR"    return TYPE_CHAR;
"VOID"    return VOID;

{charliteral}    return CHAR_LITERAL;
{floatliteral}   return FLOAT_LITERAL;
{integerliteral} return INTEGER_LITERAL;
{id}             return ID;
{stringconst}    return STRINGCONST;

"+"       return ADDOP_ADD;
"-"       return ADDOP_SUB;
"||"      return ADDOP_OR;

"*"       return MULOP_MUL;
"/"       return MULOP_DIV;
"%"       return MULOP_MOD;
"&&"      return MULOP_AND;

"<"       return RELOP_INF;
"<="      return RELOP_INFE;
"=="      return RELOP_EQU;
">"       return RELOP_SUP;
">="      return RELOP_SUPE;
"!="      return RELOP_DIFF;
"<>"      return RELOP_DIFF;

"["       return RBRACKET;
"]"       return LBRACKET;
","       return COMMA;
":"       return COLON;
";"       return SEMICOLON;
"."       return DOT;
":="      return ASSIGN;
"="       return EQUAL;
"("       return LPARENT;
")"       return RPARENT;
"!"       return NOT;
<<EOF>>   return FEOF;

"(*"    { /* Skip comments between '(*' and '*)' */
		int c;
		while((c = yyinput()) != 0){
				if(c == '*'){
						if((c = yyinput()) == ')')
							break;
						else
						unput(c);
			}
		}
	};

{unknown}	return UNKNOWN;

%%

