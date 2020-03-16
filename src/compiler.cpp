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
#include "util/string_view.hpp"

#include <cstring>
#include <string>
#include <vector>

using namespace std::string_literals;

void Compiler::print_error_preamble() const { std::cerr << "source:" << lexer.lineno() << ": "; }

void Compiler::error(string_view s) const
{
	print_error_preamble();
	std::cerr << "error: " << s << '\n';

	print_error_preamble();
	std::cerr << "note: when reading token '" << lexer.YYText() << "'\n";

	exit(-1);
}

void Compiler::bug(string_view s) const
{
	print_error_preamble();
	std::cerr << "error: COMPILER BUG\n";

	error(s);
}

void Compiler::check_type(Type a, Type b) const
{
	if (int(a) >= int(Type::CONCEPT_BEGIN))
	{
		bug("only the second operand of TypeCheck may be a type concept");
	}

	bool match = true;

	if (b == Type::ARITHMETIC)
	{
		if (a != Type::UNSIGNED_INT)
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
		error(("incompatible types: "s + type_name(a) + ", " + type_name(b)).c_str());
	}
}

TOKEN Compiler::read_token() { return (current = TOKEN(lexer.yylex())); }

bool Compiler::match_keyword(const char* keyword) const
{
	return (current == KEYWORD && std::strcmp(lexer.YYText(), keyword) == 0);
}

Compiler::Compiler(std::istream& input, std::ostream& output) :
	lexer{input, output}, codegen{std::make_unique<CodeGen>(output)}
{}

void Compiler::operator()()
{
	codegen->begin_program();

	read_token();
	parse_program();

	if (current != FEOF)
	{
		// FIXME: this is not printing the right stuff
		error((std::string("extraneous characters at end of file: [") + std::to_string(current) + "]").c_str());
	}

	codegen->finalize_program();
}

Type Compiler::parse_identifier()
{
	const std::string name = lexer.YYText();

	const auto it = variables.find(name);
	if (it == variables.end())
	{
		error((std::string("use of undeclared identifier '") + name + '\'').c_str());
	}

	const VariableType& type = it->second;

	codegen->load_variable({name, type});
	read_token();

	return type.type;
}

Type Compiler::parse_number()
{
	codegen->load_i64(std::atoi(lexer.YYText()));
	read_token();

	return Type::UNSIGNED_INT;
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

	if (current == NUMBER)
	{
		return parse_number();
	}

	if (current == ID)
	{
		return parse_identifier();
	}

	error("expected '(', number or identifier");
}

MultiplicativeOperator Compiler::parse_multiplicative_operator()
{
	const auto it = multiplicative_operator_names.find(lexer.YYText());
	read_token();
	return it != multiplicative_operator_names.end() ? it->second : MultiplicativeOperator::WTFM;
}

Type Compiler::parse_term()
{
	const Type first_type = parse_factor();

	while (current == MULOP)
	{
		MultiplicativeOperator mulop    = parse_multiplicative_operator();
		const Type             nth_type = parse_factor();

		check_type(first_type, nth_type);

		switch (mulop)
		{
		case MultiplicativeOperator::AND:
		{
			check_type(first_type, Type::BOOLEAN);
			codegen->alu_and_bool();
			break;
		}

		case MultiplicativeOperator::MUL:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_multiply_i64();
			break;
		}

		case MultiplicativeOperator::DIV:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_divide_i64();
			break;
		}

		case MultiplicativeOperator::MOD:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_modulus_i64();
			break;
		}

		case MultiplicativeOperator::WTFM:
		default:
		{
			bug("unknown multiplicative operator");
		}
		}
	}

	return first_type;
}

AdditiveOperator Compiler::parse_additive_operator()
{
	const auto it = additive_operator_names.find(lexer.YYText());
	read_token();
	return it != additive_operator_names.end() ? it->second : AdditiveOperator::WTFA;
}

Type Compiler::parse_simple_expression()
{
	const Type first_type = parse_term();

	while (current == ADDOP)
	{
		AdditiveOperator adop     = parse_additive_operator();
		const Type       nth_type = parse_term();

		check_type(first_type, nth_type);

		switch (adop)
		{
		case AdditiveOperator::OR:
		{
			check_type(first_type, Type::BOOLEAN);
			codegen->alu_or_bool();
			break;
		}

		case AdditiveOperator::ADD:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_add_i64();
			break;
		}

		case AdditiveOperator::SUB:
		{
			check_type(first_type, Type::ARITHMETIC);
			codegen->alu_sub_i64();
			break;
		}

		case AdditiveOperator::WTFA:
		default:
		{
			bug("unknown additive operator");
		}
		}
	}

	return first_type;
}

void Compiler::parse_declaration_block()
{
	if (!match_keyword("VAR"))
	{
		return;
	}

	do
	{
		std::vector<std::string> current_declarations;

		do
		{
			read_token();
			current_declarations.push_back(lexer.YYText());
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

		read_token();
	} while (current == SEMICOLON);

	if (current != DOT)
	{
		error("expected '.' at end of declaration block");
	}

	read_token();
}

Type Compiler::parse_type()
{
	if (current != TYPE)
	{
		error("expected type");
	}

	const char* name = lexer.YYText();

	if (std::strcmp(name, "INTEGER") == 0)
	{
		return Type::UNSIGNED_INT;
	}

	if (std::strcmp(name, "BOOLEAN") == 0)
	{
		return Type::BOOLEAN;
	}

	bug("unrecognized type");
}

RelationalOperator Compiler::parse_relational_operator()
{
	const auto it = relational_operator_names.find(lexer.YYText());
	read_token();
	return it != relational_operator_names.end() ? it->second : RelationalOperator::WTFR;
}

Type Compiler::parse_expression()
{
	const Type first_type = parse_simple_expression();

	if (current == RELOP)
	{
		RelationalOperator oprel    = parse_relational_operator();
		const Type         nth_type = parse_simple_expression();

		check_type(first_type, nth_type);

		switch (oprel)
		{
		case RelationalOperator::EQU: codegen->alu_equal_i64(); break;
		case RelationalOperator::DIFF: codegen->alu_not_equal_i64(); break;
		case RelationalOperator::SUPE: codegen->alu_greater_equal_i64(); break;
		case RelationalOperator::INFE: codegen->alu_lower_equal_i64(); break;
		case RelationalOperator::INF: codegen->alu_lower_i64(); break;
		case RelationalOperator::SUP: codegen->alu_greater_i64(); break;
		case RelationalOperator::WTFR:
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

	const std::string name = lexer.YYText();
	const auto        it   = variables.find(name);

	if (it == variables.end())
	{
		error((std::string("variable '") + name + "' not found").c_str());
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

	if (current != KEYWORD || strcmp(lexer.YYText(), "THEN"))
	{
		error("expected 'THEN' after conditional expression of 'IF' statement");
	}

	read_token();
	parse_statement();

	if (current == KEYWORD && strcmp(lexer.YYText(), "ELSE") == 0)
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

	if (current != KEYWORD || strcmp(lexer.YYText(), "DO"))
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

	if (current != KEYWORD || strcmp(lexer.YYText(), "TO") != 0)
	{
		error("expected 'TO' after assignement in 'FOR' statement");
	}

	codegen->statement_for_post_assignment(for_statement);

	read_token();
	check_type(parse_expression(), Type::UNSIGNED_INT);

	codegen->statement_for_post_check(for_statement);

	if (current != KEYWORD || strcmp(lexer.YYText(), "DO") != 0)
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

	if (current != KEYWORD || strcmp(lexer.YYText(), "END") != 0)
	{
		error("expected 'END' to finish block statement");
	}

	read_token();
}

void Compiler::parse_display_statement()
{
	read_token();
	const Type type = parse_expression();

	switch (type)
	{
	case Type::UNSIGNED_INT:
	{
		codegen->debug_display_i64();
		break;
	}

	default:
	{
		bug("DISPLAY statement not yet implemented for this type");
	}
	}
}

void Compiler::parse_statement()
{
	if (strcmp(lexer.YYText(), "IF") == 0)
	{
		parse_if_statement();
	}
	else if (strcmp(lexer.YYText(), "WHILE") == 0)
	{
		parse_while_statement();
	}
	else if (strcmp(lexer.YYText(), "FOR") == 0)
	{
		parse_for_statement();
	}
	else if (strcmp(lexer.YYText(), "BEGIN") == 0)
	{
		parse_block_statement();
	}
	else if (strcmp(lexer.YYText(), "DISPLAY") == 0)
	{
		parse_display_statement();
	}
	else
	{
		parse_assignment_statement();
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
