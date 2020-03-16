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

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using std::cout;

using namespace std::string_literals;

void Compiler::print_error_preamble() const { std::cerr << "source:" << lexer.lineno() << ": "; }

void Compiler::error(const char* s) const
{
	print_error_preamble();
	std::cerr << "error: " << s << '\n';

	print_error_preamble();
	std::cerr << "note: when reading token '" << lexer.YYText() << "'\n";

	exit(-1);
}

void Compiler::check_type(Type a, Type b) const
{
	assert(int(a) < int(Type::CONCEPT_BEGIN) && "Only the second operand of TypeCheck may be a type concept");

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
			error("unknown multiplicative operator");
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
			error("unknown additive operator");
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

	error("unrecognized type; this is a compiler bug");
}

RelationalOperator Compiler::parse_relational_operator()
{
	const auto it = relational_operator_names.find(lexer.YYText());
	read_token();
	return it != relational_operator_names.end() ? it->second : RelationalOperator::WTFR;
}

Type Compiler::parse_expression()
{
	RelationalOperator oprel;
	const Type         first_type = parse_simple_expression();
	if (current == RELOP)
	{
		oprel               = parse_relational_operator();
		const Type nth_type = parse_simple_expression();

		check_type(first_type, nth_type);

		cout << "\tpop %rax\n";
		cout << "\tpop %rbx\n";
		cout << "\tcmpq %rax, %rbx\n";
		switch (oprel)
		{
		case RelationalOperator::EQU: cout << "\tje Vrai" << ++label_tag << "\t# If equal\n"; break;
		case RelationalOperator::DIFF: cout << "\tjne Vrai" << ++label_tag << "\t# If different\n"; break;
		case RelationalOperator::SUPE: cout << "\tjae Vrai" << ++label_tag << "\t# If above or equal\n"; break;
		case RelationalOperator::INFE: cout << "\tjbe Vrai" << ++label_tag << "\t# If below or equal\n"; break;
		case RelationalOperator::INF: cout << "\tjb Vrai" << ++label_tag << "\t# If below\n"; break;
		case RelationalOperator::SUP: cout << "\tja Vrai" << ++label_tag << "\t# If above\n"; break;
		case RelationalOperator::WTFR:
		default: error("unknown comparison operator");
		}
		cout << "\tpush $0\t\t# False\n";
		cout << "\tjmp Suite" << label_tag << '\n';
		cout << "Vrai" << label_tag << ":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True\n";
		cout << "Suite" << label_tag << ":\n";

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

	const VariableType& variable = it->second;

	read_token();
	if (current != ASSIGN)
		error("expected ':=' in variable assignment");
	read_token();
	Type type = parse_expression();
	cout << "\tpop " << name << '\n';

	check_type(type, variable.type);

	return {name, type};
}

void Compiler::parse_if_statement()
{
	read_token();
	check_type(parse_expression(), Type::BOOLEAN);

	const auto tag = ++label_tag;

	cout << "\tpopq %rax\n";
	cout << "\ttest %rax, %rax\n";
	cout << "\tjz IfFalse" << tag << '\n';
	cout << "IfTrue" << tag << ":\n";

	if (current != KEYWORD || strcmp(lexer.YYText(), "THEN"))
	{
		error("expected 'THEN' after conditional expression of 'IF' statement");
	}

	read_token();
	parse_statement();

	if (current == KEYWORD && strcmp(lexer.YYText(), "ELSE") == 0)
	{
		cout << "\tjmp Suite" << tag << '\n';
		cout << "IfFalse" << tag << ":\n";
		read_token();
		parse_statement();
	}
	else
	{
		cout << "IfFalse" << tag << ":\n";
	}

	cout << "Suite" << tag << ":\n";
}

void Compiler::parse_while_statement()
{
	const auto tag = ++label_tag;

	cout << "WhileBegin" << tag << ":\n";

	read_token();
	const Type type = parse_expression();
	check_type(type, Type::BOOLEAN);

	cout << "\tpop %rax\n";
	cout << "\ttest %rax, %rax\n";
	cout << "\tjz Suite" << tag << '\n';

	if (current != KEYWORD || strcmp(lexer.YYText(), "DO"))
	{
		error("expected 'DO' after conditional expression of 'WHILE' statement");
	}

	read_token();
	parse_statement();

	cout << "\tjmp WhileBegin" << tag << '\n';
	cout << "Suite" << tag << ":\n";
}

void Compiler::parse_for_statement()
{
	const auto tag = ++label_tag;

	read_token();
	const auto assignment = parse_assignment_statement();
	check_type(assignment.type.type, Type::UNSIGNED_INT);

	if (current != KEYWORD || strcmp(lexer.YYText(), "TO") != 0)
	{
		error("expected 'TO' after assignement in 'FOR' statement");
	}

	cout << "ForBegin" << tag << ":\n";

	read_token();
	check_type(parse_expression(), Type::UNSIGNED_INT);

	cout << "\tpop %rax\n";
	// we branch *out* if var < %rax, mind the op order in at&t
	cout << "\tcmpq " << assignment.name << ", %rax\n";
	cout << "\tjl Suite" << tag << '\n';

	if (current != KEYWORD || strcmp(lexer.YYText(), "DO") != 0)
	{
		error("expected 'DO' after max expression in 'FOR' statement");
	}

	read_token();
	parse_statement();

	cout << "\taddq $1, " << assignment.name << '\n';
	cout << "\tjmp ForBegin" << tag << '\n';
	cout << "Suite" << tag << ":\n";
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
		cout << "\tmovq $__cc_format_string_llu, %rdi\n";
		break;
	}

	default:
	{
		error("unimplemented DISPLAY statement for this type, sorry");
	}
	}

	cout << "\tpop %rsi # Value to be displayed\n";
	cout << "\tmovb $0, %al # Number of floating point parameters (varargs)\n";
	cout << "\tcall printf\n";
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
