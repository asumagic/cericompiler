#pragma once

#include "tokeniser.hpp"
#include "types.hpp"

#include <string>

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"

enum class OPREL
{
	EQU,
	DIFF,
	INF,
	SUP,
	INFE,
	SUPE,
	WTFR
};

enum OPADD
{
	ADD,
	SUB,
	OR,
	WTFA
};

enum OPMUL
{
	MUL,
	DIV,
	MOD,
	AND,
	WTFM
};

struct VariableType
{
	Type type;
};

struct VariableAssignment
{
	std::string name;
	VariableType type;
};

bool IsDeclared(const char* id);

void              PrintErrorPreamble();
[[noreturn]] void Error(const char* s);

void TypeCheck(Type a, Type b);

TOKEN read_token();

[[nodiscard]] bool match_keyword(const char* keyword);

[[nodiscard]] Type Identifier();
[[nodiscard]] Type Number();
[[nodiscard]] Type Factor();

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
[[nodiscard]] OPMUL MultiplicativeOperator();

// Term := Factor {MultiplicativeOperator Factor}
[[nodiscard]] Type Term();

// AdditiveOperator := "+" | "-" | "||"
[[nodiscard]] OPADD AdditiveOperator();

// SimpleExpression := Term {AdditiveOperator Term}
[[nodiscard]] Type SimpleExpression();

// DeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
// Declaration := Ident {"," Ident} ":" Type
void parse_declaration_block();

// Type := "INTEGER" | "BOOLEAN"
[[nodiscard]] Type parse_type();

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
[[nodiscard]] OPREL RelationalOperator();

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
[[nodiscard]] Type Expression();

// AssignementStatement := Identifier ":=" Expression
VariableAssignment AssignementStatement();

// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement();

// WhileStatement := "WHILE" Expression DO Statement
void WhileStatement();

// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
void ForStatement();

// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement();

// DisplayStatement := "DISPLAY" Expression
void DisplayStatement();

// Statement := AssignementStatement
void Statement();

// StatementPart := Statement {";" Statement} "."
void StatementPart();

// Program := [DeclarationPart] StatementPart
void Program();
