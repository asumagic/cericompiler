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
#include "tokeniser.hpp"
#include "util/enums.hpp"
#include "util/string_view.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <vector>

void Compiler::error(string_view s) const
{
	const auto source_context = fmt::format("source:{}: ", lexer.lineno());
	fmt::print(stderr, fg(fmt::color::red), "{}error: {}\n", source_context, s.str());
	fmt::print(stderr, fg(fmt::color::red), "{}note:  while reading token '{}'\n", source_context, token_text().str());

	exit(-1);
}

void Compiler::bug(string_view s) const
{
	fmt::print(stderr, fg(fmt::color::red), "source:{}: error: COMPILER BUG!\n", lexer.lineno());

	error(s);
}

void Compiler::check_type(Type a, Type b) const
{
	if (check_enum_range(a, Type::FIRST_CONCEPT, Type::LAST_CONCEPT))
	{
		bug("only the second operand of TypeCheck may be a type concept");
	}

	bool match = true;

	if (b == Type::ARITHMETIC)
	{
		if (!check_enum_range(a, Type::FIRST_ARITHMETIC, Type::LAST_ARITHMETIC))
		{
			match = false;
		}
	}
	else if (a != b)
	{
		match = false;
	}

	if (!match)
	{
		error(fmt::format("incompatible types: {}, {}", type_name(a).str(), type_name(b).str()));
	}
}

TOKEN Compiler::read_token() { return (current = TOKEN(lexer.yylex())); }

Compiler::Compiler(std::istream& input, std::ostream& output) :
	lexer{input, output}, codegen{std::make_unique<CodeGen>(output)}
{}

void Compiler::operator()()
{
	try
	{
		codegen->begin_program();

		read_token();
		parse_program();

		if (current != FEOF)
		{
			// FIXME: this is not printing the right stuff
			error(fmt::format("extraneous characters at end of file: [{}]", current));
		}

		codegen->finalize_program();
	}
	catch (const std::runtime_error& e)
	{
		error(e.what());
	}
}

bool Compiler::is_token_keyword() const { return check_enum_range(current, TOKEN::FIRST_KEYWORD, TOKEN::LAST_KEYWORD); }
bool Compiler::is_token_type() const { return check_enum_range(current, TOKEN::FIRST_TYPE, TOKEN::LAST_TYPE); }
bool Compiler::is_token_addop() const { return check_enum_range(current, TOKEN::FIRST_ADDOP, TOKEN::LAST_ADDOP); }
bool Compiler::is_token_mulop() const { return check_enum_range(current, TOKEN::FIRST_MULOP, TOKEN::LAST_MULOP); }
bool Compiler::is_token_relop() const { return check_enum_range(current, TOKEN::FIRST_RELOP, TOKEN::LAST_RELOP); }

string_view Compiler::token_text() const { return lexer.YYText(); }

Type Compiler::parse_identifier()
{
	const std::string name = token_text();

	const auto it = variables.find(name);
	if (it == variables.end())
	{
		error(fmt::format("use of undeclared identifier '{}'", name));
	}

	const VariableType& type = it->second;

	codegen->load_variable({name, type});
	read_token();

	return type.type;
}

Type Compiler::parse_character_literal()
{
	// 2nd character in e.g. `'h'`
	codegen->load_i64(token_text()[1]);
	read_token();

	return Type::CHAR;
}

Type Compiler::parse_integer_literal()
{
	codegen->load_i64(std::stoull(token_text()));
	read_token();

	return Type::UNSIGNED_INT;
}

Type Compiler::parse_float_literal()
{
	static_assert(sizeof(double) == sizeof(std::int64_t), "double must be 64-bit on the compiler platform");

	double        source = std::stod(token_text());
	std::uint64_t target;
	std::memcpy(&target, &source, sizeof(target));

	codegen->load_i64(target);
	read_token();

	return Type::DOUBLE;
}

Type Compiler::parse_factor()
{
	// TODO: implement boolean negation '!'

	if (current == RPARENT)
	{
		read_token();
		Type type = parse_expression();
		if (current != LPARENT)
		{
			error("expected ')'");
		}
		else
		{
			read_token();
		}

		return type;
	}

	if (current == CHAR_LITERAL)
	{
		return parse_character_literal();
	}

	if (current == INTEGER_LITERAL)
	{
		return parse_integer_literal();
	}

	if (current == FLOAT_LITERAL)
	{
		return parse_float_literal();
	}

	if (current == ID)
	{
		return parse_identifier();
	}

	if (is_token_type())
	{
		return parse_type_cast();
	}

	error("expected '(', number or identifier");
}

Type Compiler::parse_type_cast()
{
	const Type destination_type = parse_type();

	if (current != RPARENT)
	{
		error("expected '(' after type for explicit type conversion");
	}

	read_token();
	const Type source_type = parse_expression();

	if (current != LPARENT)
	{
		error("expected ')' after expression for explicit type conversion");
	}
	read_token();

	codegen->convert(source_type, destination_type);

	// right now just yolo it and don't convert
	return destination_type;
}

Type Compiler::parse_term()
{
	const Type first_type = parse_factor();
	while (is_token_mulop())
	{
		const TOKEN op_token = current;
		read_token();

		const Type nth_type = parse_factor();
		check_type(first_type, nth_type);

		switch (op_token)
		{
		case TOKEN::MULOP_AND:
		{
			check_type(first_type, Type::BOOLEAN);
			codegen->alu_and_bool();
			break;
		}

		case TOKEN::MULOP_MUL:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_multiply(first_type);
			break;
		}

		case TOKEN::MULOP_DIV:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_divide(first_type);
			break;
		}

		case TOKEN::MULOP_MOD:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_modulus(first_type);
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

	while (is_token_addop())
	{
		const TOKEN op_token = current;
		read_token();

		const Type nth_type = parse_term();
		check_type(first_type, nth_type);

		switch (op_token)
		{
		case TOKEN::ADDOP_OR:
		{
			check_type(first_type, Type::BOOLEAN);
			codegen->alu_or_bool();
			break;
		}

		case TOKEN::ADDOP_ADD:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_add(first_type);
			break;
		}

		case TOKEN::ADDOP_SUB:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_sub(first_type);
			break;
		}

		default: bug("unimplemented additive operator");
		}
	}

	return first_type;
}

void Compiler::parse_declaration_block()
{
	if (current != TOKEN::KEYWORD_VAR)
	{
		return;
	}

	do
	{
		std::vector<std::string> current_declarations;

		do
		{
			read_token();
			current_declarations.push_back(token_text());
			read_token();
		} while (current == COMMA);

		if (current != COLON)
		{
			error("expected ':' after variable name list in declaration block");
		}

		read_token();

		const Type type = parse_type();

		for (auto& name : current_declarations)
		{
			variables.emplace(std::move(name), VariableType{type});
		}
	} while (current == SEMICOLON);

	if (current != DOT)
	{
		error("expected '.' at end of declaration block");
	}

	read_token();
}

Type Compiler::parse_type()
{
	if (!is_token_type())
	{
		error("expected type");
	}

	const TOKEN token = current;

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

	if (is_token_relop())
	{
		const TOKEN op_token = current;
		read_token();

		const Type nth_type = parse_simple_expression();
		check_type(first_type, nth_type);

		switch (op_token)
		{
		case TOKEN::RELOP_EQU: codegen->alu_equal(first_type); break;
		case TOKEN::RELOP_DIFF: codegen->alu_not_equal(first_type); break;
		case TOKEN::RELOP_SUPE: codegen->alu_greater_equal(first_type); break;
		case TOKEN::RELOP_INFE: codegen->alu_lower_equal(first_type); break;
		case TOKEN::RELOP_INF: codegen->alu_lower(first_type); break;
		case TOKEN::RELOP_SUP: codegen->alu_greater(first_type); break;
		default: bug("unknown comparison operator");
		}

		return Type::BOOLEAN;
	}

	return first_type;
}

Variable Compiler::parse_assignment_statement()
{
	if (current != ID)
		error("expected an identifier");

	const std::string name = token_text();
	const auto        it   = variables.find(name);

	if (it == variables.end())
	{
		error(fmt::format("assignment of undeclared variable '{}'", name));
	}

	const VariableType& variable_type = it->second;

	read_token();
	if (current != ASSIGN)
		error("expected ':=' in variable assignment");
	read_token();
	Type type = parse_expression();

	codegen->store_variable({name, variable_type});

	check_type(type, variable_type.type);

	return {name, type};
}

void Compiler::parse_if_statement()
{
	auto if_statement = codegen->statement_if_prepare();

	read_token();
	check_type(parse_expression(), Type::BOOLEAN);

	codegen->statement_if_post_check(if_statement);

	if (current != TOKEN::KEYWORD_THEN)
	{
		error("expected 'THEN' after conditional expression of 'IF' statement");
	}

	read_token();
	parse_statement();

	if (current == TOKEN::KEYWORD_ELSE)
	{
		codegen->statement_if_with_else(if_statement);
		read_token();
		parse_statement();
	}
	else
	{
		codegen->statement_if_without_else(if_statement);
	}

	codegen->statement_if_finalize(if_statement);
}

void Compiler::parse_while_statement()
{
	auto while_statement = codegen->statement_while_prepare();

	read_token();
	const Type type = parse_expression();
	check_type(type, Type::BOOLEAN);

	codegen->statement_while_post_check(while_statement);

	if (current != TOKEN::KEYWORD_DO)
	{
		error("expected 'DO' after conditional expression of 'WHILE' statement");
	}

	read_token();
	parse_statement();

	codegen->statement_while_finalize(while_statement);
}

void Compiler::parse_for_statement()
{
	read_token();
	const auto assignment = parse_assignment_statement();
	check_type(assignment.type.type, Type::UNSIGNED_INT);

	auto for_statement = codegen->statement_for_prepare(assignment);

	if (current != TOKEN::KEYWORD_TO)
	{
		error("expected 'TO' after assignement in 'FOR' statement");
	}

	codegen->statement_for_post_assignment(for_statement);

	read_token();
	check_type(parse_expression(), Type::UNSIGNED_INT);

	codegen->statement_for_post_check(for_statement);

	if (current != TOKEN::KEYWORD_DO)
	{
		error("expected 'DO' after max expression in 'FOR' statement");
	}

	read_token();
	parse_statement();

	codegen->statement_for_finalize(for_statement);
}

void Compiler::parse_block_statement()
{
	do
	{
		read_token();
		parse_statement();
	} while (current == SEMICOLON);

	if (current != TOKEN::KEYWORD_END)
	{
		error("expected 'END' to finish block statement");
	}

	read_token();
}

void Compiler::parse_display_statement()
{
	read_token();
	const Type type = parse_expression();

	codegen->debug_display(type);
}

void Compiler::parse_statement()
{
	switch (current)
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
	codegen->begin_executable_section();
	codegen->begin_main_procedure();

	parse_statement();
	while (current == SEMICOLON)
	{
		read_token();
		parse_statement();
	}

	if (current != DOT)
	{
		error("expected '.' (did you forget a ';'?)");
	}

	read_token();

	codegen->finalize_main_procedure();
	codegen->finalize_executable_section();
}

void Compiler::parse_program()
{
	codegen->begin_global_data_section();
	parse_declaration_block();
	codegen->finalize_global_data_section();

	for (const auto& it : variables)
	{
		const auto&         name = it.first;
		const VariableType& type = it.second;

		codegen->define_global_variable({name, type});
	}

	parse_statement_part();
}
