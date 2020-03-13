#pragma once

#include "operators.hpp"
#include "tokeniser.hpp"
#include "types.hpp"

#include <string>
#include <unordered_map>

#include <FlexLexer.h>

struct VariableType
{
	Type type;
};

struct VariableAssignment
{
	std::string  name;
	VariableType type;
};

class Compiler
{
	public:
	void operator()();

	private:
	TOKEN current; // Current token

	yyFlexLexer lexer;

	std::unordered_map<std::string, VariableType> variables;
	unsigned long                                 label_tag = 0;

	// Letter := "a"|...|"z"
	[[nodiscard]] Type parse_identifier();

	// Number := Digit{Digit}
	// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
	[[nodiscard]] Type parse_number();

	// Factor := Number | Letter | "(" Expression ")"| "!" Factor
	[[nodiscard]] Type parse_factor();

	// MultiplicativeOperator := "*" | "/" | "%" | "&&"
	[[nodiscard]] MultiplicativeOperator parse_multiplicative_operator();

	// Term := Factor {MultiplicativeOperator Factor}
	[[nodiscard]] Type parse_term();

	// AdditiveOperator := "+" | "-" | "||"
	[[nodiscard]] AdditiveOperator parse_additive_operator();

	// SimpleExpression := Term {AdditiveOperator Term}
	[[nodiscard]] Type parse_simple_expression();

	// DeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
	// Declaration := Ident {"," Ident} ":" Type
	void parse_declaration_block();

	// Type := "INTEGER" | "BOOLEAN"
	[[nodiscard]] Type parse_type();

	// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
	[[nodiscard]] RelationalOperator parse_relational_operator();

	// Expression := SimpleExpression [RelationalOperator SimpleExpression]
	[[nodiscard]] Type parse_expression();

	// AssignementStatement := Identifier ":=" Expression
	VariableAssignment parse_assignment_statement();

	// IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
	void parse_if_statement();

	// WhileStatement := "WHILE" Expression DO Statement
	void parse_while_statement();

	// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
	void parse_for_statement();

	// BlockStatement := "BEGIN" Statement { ";" Statement } "END"
	void parse_block_statement();

	// DisplayStatement := "DISPLAY" Expression
	void parse_display_statement();

	// Statement := AssignementStatement
	void parse_statement();

	// StatementPart := Statement {";" Statement} "."
	void parse_statement_part();

	// Program := [DeclarationPart] StatementPart
	void parse_program();

	bool is_declared(const char* id) const;

	void              print_error_preamble() const;
	[[noreturn]] void error(const char* s) const;

	void check_type(Type a, Type b) const;

	TOKEN read_token();

	[[nodiscard]] bool match_keyword(const char* keyword) const;
};
