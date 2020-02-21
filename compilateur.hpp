#pragma once

#include <string>

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};

enum IdentifierType
{
    UNSIGNED_INT
};

enum FactorType
{
    FACTOR_EXPRESSION,
    FACTOR_NUMBER,
    FACTOR_IDENTIFIER
};

bool IsDeclared(const char *id);

[[noreturn]] void Error(const std::string& s);

IdentifierType Identifier();
IdentifierType Number();
FactorType Factor();

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator();

// Term := Factor {MultiplicativeOperator Factor}
FactorType Term();

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator();

// SimpleExpression := Term {AdditiveOperator Term}
FactorType SimpleExpression();

// DeclarationPart := "[" Ident {"," Ident} "]"
void DeclarationPart();

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator();

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
FactorType Expression();

// AssignementStatement := Identifier ":=" Expression
void AssignementStatement();

// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement();

// WhileStatement := "WHILE" Expression DO Statement
void WhileStatement();

// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
void ForStatement();

// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement();

// Statement := AssignementStatement
void Statement();

// StatementPart := Statement {";" Statement} "."
void StatementPart();

// Program := [DeclarationPart] StatementPart
void Program();