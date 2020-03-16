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

	[[nodiscard]] Type parse_identifier();
	[[nodiscard]] Type parse_integer_literal();
	[[nodiscard]] Type parse_float_literal();
	[[nodiscard]] Type parse_factor();
	[[nodiscard]] Type parse_type_cast();
	[[nodiscard]] Type parse_term();
	[[nodiscard]] Type parse_simple_expression();
	void               parse_declaration_block();
	[[nodiscard]] Type parse_type();
	[[nodiscard]] Type parse_expression();
	Variable           parse_assignment_statement();
	void               parse_if_statement();
	void               parse_while_statement();
	void               parse_for_statement();
	void               parse_block_statement();
	void               parse_display_statement();
	void               parse_statement();
	void               parse_statement_part();
	void               parse_program();

	[[noreturn]] void error(string_view s) const;
	[[noreturn]] void bug(string_view s) const;

	void check_type(Type a, Type b) const;

	TOKEN read_token();
};
