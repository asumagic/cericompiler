//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "compiler.hpp"
#include "codegen.hpp"
#include "exceptions.hpp"
#include "token.hpp"
#include "util/enums.hpp"
#include "util/string_view.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <vector>

void Compiler::expect_token(TOKEN expected, string_view error_message) const
{
	if (m_current_token != expected)
	{
		error(error_message);
	}
}

void Compiler::read_token(TOKEN expected, string_view error_message)
{
	expect_token(expected, error_message);
	read_token();
}

bool Compiler::try_read_token(TOKEN expected)
{
	if (m_current_token == expected)
	{
		read_token();
		return true;
	}

	return false;
}

void Compiler::error(string_view error_message) const
{
	const auto source_context = fmt::format("source:{}: ", m_lexer.lineno());
	fmt::print(stderr, fg(fmt::color::red), "{}error: {}\n", source_context, error_message.str());
	fmt::print(stderr, fg(fmt::color::red), "{}note:  while reading token '{}'\n", source_context, token_text().str());

	exit(-1);
}

void Compiler::bug(string_view error_message) const
{
	fmt::print(stderr, fg(fmt::color::red), "source:{}: error: COMPILER BUG!\n", m_lexer.lineno());

	error(error_message);
}

void Compiler::check_type(Type a, Type b) const
{
	if (check_enum_range(a, Type::FIRST_CONCEPT, Type::LAST_CONCEPT))
	{
		bug("only the second operand of TypeCheck may be a type concept");
	}

	bool match;

	switch (b)
	{
	case Type::ARITHMETIC:
	{
		match = check_enum_range(a, Type::FIRST_ARITHMETIC, Type::LAST_ARITHMETIC);
		break;
	}

	default:
	{
		match = (a == b);
		break;
	}
	}

	if (!match)
	{
		error(fmt::format("incompatible types: {}, {}", type_name(a).str(), type_name(b).str()));
	}
}

TOKEN Compiler::read_token() { return (m_current_token = TOKEN(m_lexer.yylex())); }

Compiler::Compiler(std::istream& input, std::ostream& output) :
	m_lexer{input, output}, m_codegen{std::make_unique<CodeGen>(output)}
{}

void Compiler::operator()()
{
	try
	{
		m_codegen->begin_program();

		read_token(); // Read first token
		parse_program();

		if (m_current_token != FEOF)
		{
			error(fmt::format("extraneous characters at end of file. did you use '.' instead of ';'?"));
		}

		m_codegen->finalize_program();
	}
	catch (const CompilerError& e)
	{
		error(e.what());
	}
	catch (const std::runtime_error& e)
	{
		bug(e.what());
	}
}

string_view Compiler::token_text() const { return m_lexer.YYText(); }

Type Compiler::parse_identifier()
{
	const std::string name = token_text();

	const auto it = m_variables.find(name);
	if (it == m_variables.end())
	{
		error(fmt::format("use of undeclared identifier '{}'", name));
	}

	const VariableType& type = it->second;

	m_codegen->load_variable({name, type});
	read_token();

	return type.type;
}

Type Compiler::parse_character_literal()
{
	// 2nd character in e.g. `'h'`
	m_codegen->load_i64(token_text()[1]);
	read_token();

	return Type::CHAR;
}

Type Compiler::parse_integer_literal()
{
	static_assert(
		sizeof(unsigned long long) >= sizeof(std::int64_t),
		"unsigned long long must be 64-bit on the compiler platform");

	m_codegen->load_i64(std::stoull(token_text()));
	read_token();

	return Type::UNSIGNED_INT;
}

Type Compiler::parse_float_literal()
{
	static_assert(sizeof(double) == sizeof(std::int64_t), "double must be 64-bit on the compiler platform");

	double        source = std::stod(token_text());
	std::uint64_t target;
	std::memcpy(&target, &source, sizeof(target));

	m_codegen->load_i64(target);
	read_token();

	return Type::DOUBLE;
}

Type Compiler::parse_factor()
{
	// TODO: implement boolean negation '!'

	switch (m_current_token)
	{
	case LPARENT:
	{
		read_token();
		Type type = parse_expression();
		read_token(RPARENT, "expected ')'");

		return type;
	}

	case CHAR_LITERAL: return parse_character_literal();
	case INTEGER_LITERAL: return parse_integer_literal();
	case FLOAT_LITERAL: return parse_float_literal();
	case ID: return parse_identifier();

	default:
		if (is_token_type(m_current_token))
		{
			return parse_type_cast();
		}
	}

	error("expected '(', number or identifier");
}

Type Compiler::parse_type_cast()
{
	const Type destination_type = parse_type();
	read_token(LPARENT, "expected '('");
	const Type source_type = parse_expression();
	read_token(RPARENT, "expected ')' after expression for explicit type conversion");

	m_codegen->convert(source_type, destination_type);

	// right now just yolo it and don't convert
	return destination_type;
}

Type Compiler::parse_term()
{
	const Type first_type = parse_factor();
	while (is_token_mulop(m_current_token))
	{
		const TOKEN op_token = m_current_token;
		read_token();

		const Type nth_type = parse_factor();
		check_type(first_type, nth_type);

		switch (op_token)
		{
		case TOKEN::MULOP_AND:
		{
			check_type(first_type, Type::BOOLEAN);
			m_codegen->alu_and_bool();
			break;
		}

		case TOKEN::MULOP_MUL:
		{
			check_type(first_type, Type::ARITHMETIC);
			m_codegen->alu_multiply(first_type);
			break;
		}

		case TOKEN::MULOP_DIV:
		{
			check_type(first_type, Type::ARITHMETIC);
			m_codegen->alu_divide(first_type);
			break;
		}

		case TOKEN::MULOP_MOD:
		{
			check_type(first_type, Type::ARITHMETIC);
			m_codegen->alu_modulus(first_type);
			break;
		}

		default: bug("unimplemented multiplicative operator");
		}
	}

	return first_type;
}

Type Compiler::parse_simple_expression()
{
	const Type first_type = parse_term();

	while (is_token_addop(m_current_token))
	{
		const TOKEN op_token = m_current_token;
		read_token();

		const Type nth_type = parse_term();
		check_type(first_type, nth_type);

		switch (op_token)
		{
		case TOKEN::ADDOP_OR:
		{
			check_type(first_type, Type::BOOLEAN);
			m_codegen->alu_or_bool();
			break;
		}

		case TOKEN::ADDOP_ADD:
		{
			check_type(first_type, Type::ARITHMETIC);
			m_codegen->alu_add(first_type);
			break;
		}

		case TOKEN::ADDOP_SUB:
		{
			check_type(first_type, Type::ARITHMETIC);
			m_codegen->alu_sub(first_type);
			break;
		}

		default: bug("unimplemented additive operator");
		}
	}

	return first_type;
}

void Compiler::parse_declaration_block()
{
	if (m_current_token != TOKEN::KEYWORD_VAR)
	{
		return;
	}

	do
	{
		std::vector<std::string> current_declarations;

		do
		{
			read_token(); // Skip VAR or previous COMMA
			current_declarations.push_back(token_text());
			read_token(); // Skip variable name
		} while (m_current_token == COMMA);

		read_token(COLON, "expected ':' after variable name list in declaration block");

		const Type type = parse_type();

		for (auto& name : current_declarations)
		{
			m_variables.emplace(std::move(name), VariableType{type});
		}
	} while (m_current_token == SEMICOLON);

	read_token(DOT, "expected '.' at end of declaration block");

	for (const auto& it : m_variables)
	{
		const auto&         name = it.first;
		const VariableType& type = it.second;

		m_codegen->define_global_variable({name, type});
	}
}

Type Compiler::parse_type()
{
	if (!is_token_type(m_current_token))
	{
		error("expected type");
	}

	const TOKEN token = m_current_token;

	read_token();

	switch (token)
	{
	case TOKEN::TYPE_INTEGER: return Type::UNSIGNED_INT;
	case TOKEN::TYPE_DOUBLE: return Type::DOUBLE;
	case TOKEN::TYPE_BOOLEAN: return Type::BOOLEAN;
	case TOKEN::TYPE_CHAR: return Type::CHAR;
	default: bug("unrecognized type");
	}
}

Type Compiler::parse_expression()
{
	const Type first_type = parse_simple_expression();

	if (is_token_relop(m_current_token))
	{
		const TOKEN op_token = m_current_token;
		read_token();

		const Type nth_type = parse_simple_expression();
		check_type(first_type, nth_type);

		switch (op_token)
		{
		case TOKEN::RELOP_EQU: m_codegen->alu_equal(first_type); break;
		case TOKEN::RELOP_DIFF: m_codegen->alu_not_equal(first_type); break;
		case TOKEN::RELOP_SUPE: m_codegen->alu_greater_equal(first_type); break;
		case TOKEN::RELOP_INFE: m_codegen->alu_lower_equal(first_type); break;
		case TOKEN::RELOP_INF: m_codegen->alu_lower(first_type); break;
		case TOKEN::RELOP_SUP: m_codegen->alu_greater(first_type); break;
		default: bug("unknown comparison operator");
		}

		return Type::BOOLEAN;
	}

	return first_type;
}

Variable Compiler::parse_assignment_statement()
{
	expect_token(ID, "expected an identifier");

	const std::string name = token_text();
	const auto        it   = m_variables.find(name);

	if (it == m_variables.end())
	{
		error(fmt::format("assignment of undeclared variable '{}'", name));
	}

	const VariableType& variable_type = it->second;

	read_token(); // We needed the token_text up until now - consume the identifier
	read_token(ASSIGN, "expected ':=' in variable assignment");

	Type type = parse_expression();

	m_codegen->store_variable({name, variable_type});

	check_type(type, variable_type.type);

	return {name, {type}};
}

void Compiler::parse_if_statement()
{
	auto if_statement = m_codegen->statement_if_prepare();

	read_token();
	check_type(parse_expression(), Type::BOOLEAN);

	read_token(KEYWORD_THEN, "expected 'THEN' after conditional expression of 'IF' statement");

	m_codegen->statement_if_post_check(if_statement);

	parse_statement();

	if (try_read_token(KEYWORD_ELSE))
	{
		m_codegen->statement_if_with_else(if_statement);
		parse_statement();
	}
	else
	{
		m_codegen->statement_if_without_else(if_statement);
	}

	m_codegen->statement_if_finalize(if_statement);
}

void Compiler::parse_while_statement()
{
	auto while_statement = m_codegen->statement_while_prepare();

	read_token();
	const Type type = parse_expression();
	check_type(type, Type::BOOLEAN);

	read_token(KEYWORD_DO, "expected 'DO' after conditional expression of 'WHILE' statement");

	m_codegen->statement_while_post_check(while_statement);

	parse_statement();

	m_codegen->statement_while_finalize(while_statement);
}

void Compiler::parse_for_statement()
{
	read_token();
	const auto assignment = parse_assignment_statement();
	check_type(assignment.type.type, Type::UNSIGNED_INT);

	auto for_statement = m_codegen->statement_for_prepare(assignment);
	m_codegen->statement_for_post_assignment(for_statement);

	read_token(KEYWORD_TO, "expected 'TO' after assignement in 'FOR' statement");

	check_type(parse_expression(), Type::UNSIGNED_INT);

	read_token(KEYWORD_DO, "expected 'DO' after max expression in 'FOR' statement");

	m_codegen->statement_for_post_check(for_statement);

	parse_statement();

	m_codegen->statement_for_finalize(for_statement);
}

void Compiler::parse_block_statement()
{
	do
	{
		read_token();
		parse_statement();
	} while (m_current_token == SEMICOLON);

	read_token(KEYWORD_END, "expected 'END' to finish block statement");
}

void Compiler::parse_display_statement()
{
	read_token();
	const Type type = parse_expression();

	m_codegen->debug_display(type);
}

void Compiler::parse_statement()
{
	switch (m_current_token)
	{
	case TOKEN::KEYWORD_IF: parse_if_statement(); break;
	case TOKEN::KEYWORD_WHILE: parse_while_statement(); break;
	case TOKEN::KEYWORD_FOR: parse_for_statement(); break;
	case TOKEN::KEYWORD_BEGIN: parse_block_statement(); break;
	case TOKEN::KEYWORD_DISPLAY: parse_display_statement(); break;
	default: parse_assignment_statement(); break;
	}
}

void Compiler::parse_statement_part()
{
	m_codegen->begin_executable_section();
	m_codegen->begin_main_procedure();

	parse_statement();
	while (m_current_token == SEMICOLON)
	{
		read_token();
		parse_statement();
	}

	read_token(DOT, "expected '.' (did you forget a ';'?)");

	m_codegen->finalize_main_procedure();
	m_codegen->finalize_executable_section();
}

void Compiler::parse_program()
{
	m_codegen->begin_global_data_section();
	parse_declaration_block();
	m_codegen->finalize_global_data_section();

	parse_statement_part();
}
