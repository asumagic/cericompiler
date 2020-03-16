#pragma once

#include "codegen.hpp"
#include "tokeniser.hpp"
#include "types.hpp"
#include "util/string_view.hpp"

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

#include <FlexLexer.h>

struct VariableType
{
	// This is a struct on its own in case we add metadata or anything of the sort
	Type type;
};

struct Variable
{
	std::string  name;
	VariableType type;
};

class Compiler
{
	public:
	Compiler(std::istream& = std::cin, std::ostream& = std::cout);

	void operator()();

	private:
	TOKEN current; // Current token

	yyFlexLexer lexer;

	std::unordered_map<std::string, VariableType> variables;

	std::unique_ptr<CodeGen> codegen;

	[[nodiscard]] bool is_token_keyword() const;
	[[nodiscard]] bool is_token_type() const;
	[[nodiscard]] bool is_token_addop() const;
	[[nodiscard]] bool is_token_mulop() const;
	[[nodiscard]] bool is_token_relop() const;

	[[nodiscard]] string_view token_text() const;

	// Letter := "a"|...|"z"
	[[nodiscard]] Type parse_identifier();

	// Number := Digit{Digit}
	// Digit := "0"|...|"9"
	[[nodiscard]] Type parse_number();

	// Factor := Number | Letter | "(" Expression ")"| "!" Factor
	[[nodiscard]] Type parse_factor();

	[[nodiscard]] Type parse_type_cast();

	// Term := Factor {MultiplicativeOperator Factor}
	// MultiplicativeOperator := "*" | "/" | "%" | "&&"
	[[nodiscard]] Type parse_term();

	// SimpleExpression := Term {AdditiveOperator Term}
	// AdditiveOperator := "+" | "-" | "||"
	[[nodiscard]] Type parse_simple_expression();

	// DeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
	// VarDeclaration := Ident {"," Ident} ":" Type
	void parse_declaration_block();

	// Type := "INTEGER" | "BOOLEAN"
	[[nodiscard]] Type parse_type();

	// Expression := SimpleExpression [RelationalOperator SimpleExpression]
	// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
	[[nodiscard]] Type parse_expression();

	// AssignementStatement := Identifier ":=" Expression
	Variable parse_assignment_statement();

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

	[[noreturn]] void error(string_view s) const;
	[[noreturn]] void bug(string_view s) const;

	void check_type(Type a, Type b) const;

	TOKEN read_token();
};
