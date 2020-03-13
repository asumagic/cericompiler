#pragma once

// tokeniser.h : shared definition for tokeniser.l and compilateur.cpp

enum TOKEN
{
	FEOF,
	UNKNOWN,
	NUMBER,
	ID,
	TYPE,
	STRINGCONST,
	RBRACKET,
	LBRACKET,
	RPARENT,
	LPARENT,
	COMMA,
	COLON,
	SEMICOLON,
	DOT,
	ADDOP,
	MULOP,
	RELOP,
	NOT,
	ASSIGN,
	KEYWORD
};
