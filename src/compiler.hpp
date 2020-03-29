#pragma once

#include "ast/nodes/all.hpp"
#include "ast/visitors/debugprint.hpp"
#include "codegen/x86/codegen.hpp"
#include "function.hpp"
#include "token.hpp"
#include "types.hpp"
#include "usertype.hpp"
#include "util/enums.hpp"
#include "util/string_view.hpp"
#include "variable.hpp"

#include <iosfwd>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <FlexLexer.h>

using BinaryOperator = ast::nodes::BinaryExpression::Operator;

class Compiler
{
	friend class CodeGen;

	public:
	enum class Target
	{
		APPLE_DARWIN,
		LINUX
	};

	struct Config
	{
		std::vector<std::string> include_lookup_paths;
		Target                   target;
	};

	Compiler(
		const Config& config,
		string_view   file_name = "<stdin>.pas",
		std::istream&           = std::cin,
		std::ostream&           = std::cout);

	void operator()();

	private:
	const Config& m_config;

	std::stack<std::string> m_file_name_stack;

	std::ostream& m_output_stream;

	std::unique_ptr<yyFlexLexer> m_lexer;
	TOKEN                        m_current_token;

	std::unordered_set<std::string> m_includes;

	std::unique_ptr<CodeGen> m_codegen;

	Type m_first_free_type = Type::FIRST_USER_DEFINED;

	int operator_priority(BinaryOperator op) const;

	void               parse_include();                     // todo: should it be in ast?
	[[nodiscard]] Type parse_type(bool allow_void = false); // todo: determine how to do

	char           read_character_literal();
	std::uint64_t  read_integer_literal();
	double         read_float_literal();
	std::string    read_string_literal();
	std::string    read_identifier();
	BinaryOperator try_read_binop();

	std::unique_ptr<ast::nodes::Expression> _parse_character_literal();
	std::unique_ptr<ast::nodes::Expression> _parse_integer_literal();
	std::unique_ptr<ast::nodes::Expression> _parse_float_literal();
	std::unique_ptr<ast::nodes::Expression> _parse_string_literal();

	std::unique_ptr<ast::nodes::VariableDeclarationBlock>   _parse_variable_declaration_block();
	std::unique_ptr<ast::nodes::ForeignFunctionDeclaration> _parse_foreign_function_declaration();

	std::unique_ptr<ast::nodes::Statement> _parse_if_statement();
	std::unique_ptr<ast::nodes::Statement> _parse_while_statement();
	std::unique_ptr<ast::nodes::Statement> _parse_for_statement();
	std::unique_ptr<ast::nodes::Statement> _parse_block_statement();
	std::unique_ptr<ast::nodes::Statement> _parse_display_statement();
	std::unique_ptr<ast::nodes::Statement> _parse_statement();

	std::unique_ptr<ast::nodes::Expression> _parse_type_cast();

	std::unique_ptr<ast::nodes::Expression> _parse_primary();
	std::unique_ptr<ast::nodes::Expression> _parse_unary();
	std::unique_ptr<ast::nodes::Expression>
											_parse_binop_rhs(std::unique_ptr<ast::nodes::Expression> lhs, int priority = 0);
	std::unique_ptr<ast::nodes::Expression> _parse_expression();
	std::unique_ptr<ast::nodes::Node>       _parse_program();

	Type create_type(UserType user_type);
	Type allocate_type_id();

	string_view current_file() const;

	void show_source_context() const;

	//! \brief Halt the execution of the compiler due to an ill-formed program and display details provided by \p
	//! error_message.
	[[noreturn]] void error(string_view error_message) const;

	//! \brief Provide a note providing context for a compiler diagnostic.
	void note(string_view note_message) const;

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

	[[nodiscard]] string_view token_text() const;

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
