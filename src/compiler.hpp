#pragma once

#include "codegen/x86/codegen.hpp"
#include "token.hpp"
#include "types.hpp"
#include "util/string_view.hpp"
#include "variable.hpp"

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>

#include <FlexLexer.h>

class Compiler
{
	public:
	Compiler(string_view unit_name = "<stdin>", std::istream& = std::cin, std::ostream& = std::cout);

	void operator()();

	private:
	std::string m_unit_name;

	yyFlexLexer m_lexer;
	TOKEN       m_current_token;

	std::unordered_map<std::string, VariableType> m_variables;
	std::unordered_map<std::string, Type>         m_typedefs;

	std::unique_ptr<CodeGen> m_codegen;

	[[nodiscard]] string_view token_text() const;

	[[nodiscard]] Type parse_identifier();
	[[nodiscard]] Type parse_character_literal();
	[[nodiscard]] Type parse_integer_literal();
	[[nodiscard]] Type parse_float_literal();
	[[nodiscard]] Type parse_factor();
	[[nodiscard]] Type parse_type_cast();
	[[nodiscard]] Type parse_term();
	[[nodiscard]] Type parse_simple_expression();
	void               parse_declaration_block();
	void               parse_variable_declaration_block();
	[[nodiscard]] Type parse_type();
	void               parse_type_definition();
	[[nodiscard]] Type parse_expression();
	Variable           parse_assignment_statement();
	void               parse_if_statement();
	void               parse_while_statement();
	void               parse_for_statement();
	void               parse_block_statement();
	void               parse_display_statement();
	void               parse_statement();
	void               parse_declaration();
	void               parse_main_block_statement();
	void               parse_program();

	void emit_global_variables();

	std::string source_context() const;

	//! \brief Halt the execution of the compiler due to an ill-formed program and display details provided by \p
	//! error_message.
	[[noreturn]] void error(string_view error_message) const;

	//! \brief Halt the execution of the compiler due to a compiler bug and display details provided by \p
	//! error_message.
	[[noreturn]] void bug(string_view error_message) const;

	//! \brief Check whether types a and b are compatible (i.e. they behave identically) otherwise show an error.
	//!
	//! \param a may only be a concrete type, e.g. UNSIGNED_INT or CHAR.
	//! \param b may be both a concrete type and a concept.
	//!
	//! \details
	//!		Examples:
	//!		- check_type(Type::UNSIGNED_INT, Type::ARITHMETIC) will *not* show an error
	//!		- check_type(Type::UNSIGNED_INT, Type::UNSIGNED_INT) will *not* show an error
	//!		- check_type(Type::DOUBLE, Type::CHAR) *will* show an error
	//!		- check_type(Type::CHAR, Type::ARITHMETIC) *will* show an error
	//!		- check_type(Type::ARITHMETIC, Type::UNSIGNED_INT) will show a *compiler bug error*
	void check_type(Type a, Type b) const;

	//! \brief If the current token is not \p expected, show \p error_message as an error.
	void expect_token(TOKEN expected, string_view error_message) const;

	//! \brief If the current token is \p expected, skip it, otherwise show \p error_message as an error.
	void read_token(TOKEN expected, string_view error_message);

	//! \brief If the current token is \p expected, read it and return true, otherwise return false.
	bool try_read_token(TOKEN expected);

	//! \brief Read the next token and update \var current.
	//! \warning This will invalidate any prior token_text() references.
	TOKEN read_token();
};
