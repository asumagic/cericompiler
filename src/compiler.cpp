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
#include "ast/visitors/debugprint.hpp"
#include "exceptions.hpp"
#include "token.hpp"
#include "util/enums.hpp"
#include "util/string_view.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>
#include <vector>

Compiler::Compiler(const Config& config, string_view file_name, std::istream& input, std::ostream& output) :
	m_config{config}, m_output_stream{output}, m_lexer{new yyFlexLexer(input, output)}
{
	m_file_name_stack.push(file_name);

	if (!input)
	{
		error("could not open source for writing");
	}

	if (!output)
	{
		error("could not open destination for writing");
	}
}

void Compiler::operator()()
{
	try
	{
		/*
		 m_codegen->begin_program();

		 read_token(); // Read first token
		 parse_program();

		 if (m_current_token != FEOF)
		 {
			 error(fmt::format("extraneous characters at end of file. did you use '.' instead of ';'?"));
		 }

		 m_codegen->finalize_program();
		 */

		read_token();
		auto program = parse_program();

		ast::visitors::DebugPrint visitor;
		program->visit(visitor);

		error("done");
	}
	catch (const CompilerError& e)
	{
		// Rethrow the exception. This is done so that we don't call bug() on a compiler error caught by the next
		// handler.
		throw;
	}
	catch (const std::runtime_error& e)
	{
		bug(e.what());
	}
}

std::unique_ptr<ast::nodes::TypeName> Compiler::parse_type(bool allow_void)
{
	if (allow_void && try_read_token(VOID))
	{
		return std::make_unique<ast::nodes::BuiltinType>(Type::VOID);
	}

	if (is_token_type(m_current_token))
	{
		const TOKEN token = m_current_token;

		read_token();

		switch (token)
		{
		case TOKEN::TYPE_INTEGER: return std::make_unique<ast::nodes::BuiltinType>(Type::UNSIGNED_INT);
		case TOKEN::TYPE_DOUBLE: return std::make_unique<ast::nodes::BuiltinType>(Type::DOUBLE);
		case TOKEN::TYPE_BOOLEAN: return std::make_unique<ast::nodes::BuiltinType>(Type::BOOLEAN);
		case TOKEN::TYPE_CHAR: return std::make_unique<ast::nodes::BuiltinType>(Type::CHAR);
		default: bug("unrecognized type");
		}
	}

	if (m_current_token == ID)
	{
		bug("aaaaaa");
		/*const auto it = m_typedefs.find(token_text());

		if (it == m_typedefs.end())
		{
			error("expected type but identifier does not refer to any type definition");
		}

		read_token();

		return it->second;*/
	}

	if (try_read_token(EXPONENT))
	{
		/*UserType type(UserType::Category::POINTER);
		type.layout_data.pointer.target = parse_type(false);

		return create_type(type);*/
		bug("bbbbb");
	}

	error("expected type");
}

int Compiler::operator_priority(BinaryOperator op) const
{
	// TODO: move this map elsewhere
	constexpr int equ_priority = 10, add_priority = 20, mul_priority = 30;

	static const std::unordered_map<BinaryOperator, int, EnumClassHash> map{
		{BinaryOperator::INVALID, -1},

		{BinaryOperator::ADD, add_priority},
		{BinaryOperator::SUBTRACT, add_priority},
		{BinaryOperator::MULTIPLY, mul_priority},
		{BinaryOperator::DIVIDE, mul_priority},
		{BinaryOperator::MODULUS, mul_priority},

		{BinaryOperator::EQUAL, equ_priority},
		{BinaryOperator::NOT_EQUAL, equ_priority},
		{BinaryOperator::GREATER_EQUAL, equ_priority},
		{BinaryOperator::LOWER_EQUAL, equ_priority},
		{BinaryOperator::GREATER, equ_priority},
		{BinaryOperator::LOWER, equ_priority},

		{BinaryOperator::LOGICAL_AND, mul_priority},
		{BinaryOperator::LOGICAL_OR, add_priority}};

	return map.find(op)->second;
}

char Compiler::read_character_literal()
{
	const char value = token_text()[1];
	read_token();

	return value;
}

uint64_t Compiler::read_integer_literal()
{
	static_assert(
		sizeof(unsigned long long) >= sizeof(std::int64_t),
		"unsigned long long must be 64-bit on the compiler platform");

	const auto value = std::stoull(token_text());
	read_token();

	return value;
}

double Compiler::read_float_literal()
{
	const double value = std::stod(token_text());
	read_token();

	return value;
}

std::string Compiler::read_string_literal()
{
	const std::string value = token_text();
	read_token();

	return value.substr(1, value.size() - 1);
}

std::string Compiler::read_identifier()
{
	expect_token(TOKEN::ID, "expected identifier");

	const std::string value = token_text();
	read_token();
	return value;
}

BinaryOperator Compiler::peek_binop()
{
	switch (m_current_token)
	{
	case TOKEN::ADDOP_ADD: return BinaryOperator::ADD;
	case TOKEN::ADDOP_SUB: return BinaryOperator::SUBTRACT;
	case TOKEN::MULOP_MUL: return BinaryOperator::MULTIPLY;
	case TOKEN::MULOP_DIV: return BinaryOperator::DIVIDE;
	case TOKEN::MULOP_MOD: return BinaryOperator::MODULUS;

	case TOKEN::RELOP_EQU: return BinaryOperator::EQUAL;
	case TOKEN::RELOP_DIFF: return BinaryOperator::NOT_EQUAL;
	case TOKEN::RELOP_SUPE: return BinaryOperator::GREATER_EQUAL;
	case TOKEN::RELOP_INFE: return BinaryOperator::LOWER_EQUAL;
	case TOKEN::RELOP_SUP: return BinaryOperator::GREATER;
	case TOKEN::RELOP_INF: return BinaryOperator::LOWER;

	case TOKEN::MULOP_AND: return BinaryOperator::LOGICAL_AND;
	case TOKEN::ADDOP_OR: return BinaryOperator::LOGICAL_OR;

	default: return BinaryOperator::INVALID;
	}
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_character_literal()
{
	return std::make_unique<ast::nodes::CharacterLiteral>(read_character_literal());
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_integer_literal()
{
	return std::make_unique<ast::nodes::IntegerLiteral>(read_integer_literal());
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_float_literal()
{
	return std::make_unique<ast::nodes::CharacterLiteral>(read_character_literal());
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_string_literal()
{
	return std::make_unique<ast::nodes::StringLiteral>(read_string_literal());
}

std::unique_ptr<ast::nodes::VariableDeclarationBlock> Compiler::parse_variable_declaration_block()
{
	read_token(TOKEN::KEYWORD_VAR, "expected 'VAR' to begin variable declaration block");

	std::vector<ast::nodes::VariableDeclarationBlock::MultipleDeclaration> multiple_declarations;

	for (;;)
	{
		std::vector<std::string> names;

		do
		{
			names.push_back(token_text());
			read_token();
		} while (try_read_token(TOKEN::COMMA));

		read_token(TOKEN::COLON, "expected ':' after variable name list in declaration block");

		auto type = parse_type();

		multiple_declarations.emplace_back(std::move(names), std::move(type));

		read_token(TOKEN::SEMICOLON, "expected ';' after multiple variable declaration");

		// If the current token is an identifier, then we assume we're continuing to parse a variable declaration.
		// e.g.:
		//     VAR
		//         a : INTEGER;
		//         b, c : BOOLEAN;
		//     BEGIN END.
		// At the end of the 1st iteration, `b` is read and is an identifier, so it loops again.
		// At the end of the 2nd iteration, `BEGIN` is read but is not an ID, therefore this will break out.
		if (m_current_token != ID)
		{
			break;
		}
	}

	return std::make_unique<ast::nodes::VariableDeclarationBlock>(std::move(multiple_declarations));
}

std::unique_ptr<ast::nodes::ForeignFunctionDeclaration> Compiler::parse_foreign_function_declaration()
{
	ast::nodes::Function function;
	function.foreign = true;

	read_token(TOKEN::KEYWORD_FFI, "expected 'FFI'");
	const std::string name = read_identifier();
	read_token(TOKEN::LPARENT, "expected '(' after function name");

	if (m_current_token != RPARENT)
	{
		do
		{
			if (is_token_type(m_current_token))
			{
				auto type = parse_type();
				function.parameters.push_back(std::move(type));
			}
		} while (try_read_token(TOKEN::COMMA));
	}

	read_token(RPARENT, "expected ')' after type list");
	read_token(COLON, "expected ':' after ')' to specify return type of foreign function");

	function.return_type = parse_type(true);

	read_token(SEMICOLON, "expected ';' after FFI declaration");

	return std::make_unique<ast::nodes::ForeignFunctionDeclaration>(std::move(name), std::move(function));
}

std::unique_ptr<ast::nodes::Include> Compiler::parse_include()
{
	read_token(KEYWORD_INCLUDE, "expected 'INCLUDE'");

	expect_token(TOKEN::STRINGCONST, "expected string literal after INCLUDE directive");
	const std::string path = token_text().substr(1, token_text().size() - 2);

	read_token(); // consume STRINGCONST include path

	read_token(SEMICOLON, "expected ';' after INCLUDE directive");

	const auto emplace_result = m_includes.emplace(path);
	const bool success        = emplace_result.second;

	if (!success)
	{
		// Already included this file
		return nullptr;
	}

	std::ifstream included_source{path};

	if (!included_source)
	{
		for (const std::string& directory : m_config.include_lookup_paths)
		{
			included_source.open(directory + '/' + path);

			if (included_source)
			{
				break;
			}
		}
	}

	if (!included_source)
	{
		try
		{
			error(fmt::format("cannot open include file '{}'", path));
		}
		catch (const CompilerError& error)
		{
			note("tried in working directory");

			for (const std::string& directory : m_config.include_lookup_paths)
			{
				note(fmt::format("tried in '{}'", directory));
			}

			throw;
		}
	}

	// Create new lexer state and save old state
	auto new_lexer_state   = std::unique_ptr<yyFlexLexer>{new yyFlexLexer(included_source, m_output_stream)};
	auto old_lexer_state   = std::move(m_lexer);
	m_lexer                = std::move(new_lexer_state);
	auto old_current_token = m_current_token;
	m_file_name_stack.push(path);

	const auto restore_state = [&] {
		m_file_name_stack.pop();
		m_lexer         = std::move(old_lexer_state);
		m_current_token = old_current_token;
	};

	try
	{
		// Read the first token using the new lexer
		read_token();

		auto included_program = parse_program();

		if (m_current_token != TOKEN::FEOF)
		{
			error("expected end of file");
		}

		return std::make_unique<ast::nodes::Include>(std::move(path), std::move(included_program));
	}
	catch (const CompilerError& e)
	{
		restore_state();
		note("included here");
		throw;
	}
}

std::unique_ptr<ast::nodes::Statement> Compiler::parse_if_statement()
{
	read_token(TOKEN::KEYWORD_IF, "expected 'IF'");

	auto conditional = parse_expression();

	read_token(TOKEN::KEYWORD_THEN, "expected 'THEN' after conditional expression of 'IF' statement");

	auto on_success = parse_statement();
	auto on_failure = try_read_token(TOKEN::KEYWORD_ELSE) ? parse_statement() : nullptr;

	return std::make_unique<ast::nodes::IfStatement>(
		std::move(conditional), std::move(on_success), std::move(on_failure));
}

std::unique_ptr<ast::nodes::Statement> Compiler::parse_while_statement()
{
	read_token(TOKEN::KEYWORD_WHILE, "expected 'WHILE'");

	auto conditional = parse_expression();

	read_token(TOKEN::KEYWORD_DO, "expected 'DO' after conditional expression of 'WHILE' statement");

	auto loop_body = parse_statement();

	return std::make_unique<ast::nodes::WhileStatement>(std::move(conditional), std::move(loop_body));
}

std::unique_ptr<ast::nodes::Statement> Compiler::parse_for_statement()
{
	read_token(TOKEN::KEYWORD_FOR, "expected 'FOR'");

	bug("todo for statement");

	/*
	read_token();
	const auto assignment = parse_assignment_statement();
	check_type(assignment.type.type, Type::UNSIGNED_INT);

	ForStatement for_statement;
	m_codegen->statement_for_prepare(for_statement, assignment);
	m_codegen->statement_for_post_assignment(for_statement);

	read_token(KEYWORD_TO, "expected 'TO' after assignement in 'FOR' statement");

	check_type(parse_expression(), Type::UNSIGNED_INT);

	read_token(KEYWORD_DO, "expected 'DO' after max expression in 'FOR' statement");

	m_codegen->statement_for_post_check(for_statement);

	parse_statement();

	m_codegen->statement_for_finalize(for_statement);*/
}

std::unique_ptr<ast::nodes::Statement> Compiler::parse_block_statement()
{
	read_token(TOKEN::KEYWORD_BEGIN, "expected 'BEGIN' to begin block statement");

	std::vector<std::unique_ptr<ast::nodes::Statement>> statements;

	do
	{
		// This enables:
		// - empty block statements, e.g. BEGIN END
		// - optional semicolons at the last statement of a block statement
		if (m_current_token == TOKEN::KEYWORD_END)
		{
			break;
		}

		statements.push_back(parse_statement());
	} while (try_read_token(TOKEN::SEMICOLON));

	read_token(TOKEN::KEYWORD_END, "expected 'END' to finish block statement");

	return std::make_unique<ast::nodes::BlockStatement>(std::move(statements));
}

std::unique_ptr<ast::nodes::Statement> Compiler::parse_display_statement()
{
	read_token(TOKEN::KEYWORD_DISPLAY, "expected 'DISPLAY'");

	auto expression = parse_expression();

	return std::make_unique<ast::nodes::DisplayStatement>(std::move(expression));
}

std::unique_ptr<ast::nodes::Statement> Compiler::parse_statement()
{
	switch (m_current_token)
	{
	case TOKEN::KEYWORD_IF: return parse_if_statement();
	case TOKEN::KEYWORD_WHILE: return parse_while_statement();
	case TOKEN::KEYWORD_FOR: return parse_for_statement();
	case TOKEN::KEYWORD_BEGIN: return parse_block_statement();
	case TOKEN::KEYWORD_DISPLAY: return parse_display_statement();
	default:
	{
		auto expression = parse_expression();

		if (expression == nullptr)
		{
			return nullptr;
		}

		if (try_read_token(TOKEN::ASSIGN))
		{
			auto rhs = parse_expression();

			if (rhs == nullptr)
			{
				error("expected rhs");
			}

			return std::make_unique<ast::nodes::AssignmentStatement>(std::move(expression), std::move(rhs));
		}

		return expression;
	}
	}
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_type_cast()
{
	read_token(TOKEN::KEYWORD_CONVERT, "expected 'CONVERT'");
	auto expression = parse_expression();

	read_token(TOKEN::KEYWORD_TO, "expected 'TO' after expression in CONVERT expression");
	auto target_type = parse_type();

	return std::make_unique<ast::nodes::TypeCastExpression>(std::move(target_type), std::move(expression));
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_primary()
{
	switch (m_current_token)
	{
	case TOKEN::CHAR_LITERAL:
	{
		return parse_character_literal();
		break;
	}

	case TOKEN::FLOAT_LITERAL:
	{
		return parse_float_literal();
		break;
	}

	case TOKEN::INTEGER_LITERAL:
	{
		return parse_integer_literal();
		break;
	}

	case TOKEN::ID:
	{
		const auto identifier = read_identifier();

		if (try_read_token(TOKEN::LPARENT))
		{
			// Function call
			// TODO: determine how to move this to a function. problem is having consumed identifier already

			std::vector<std::unique_ptr<ast::nodes::Expression>> arguments;

			if (m_current_token != TOKEN::RPARENT)
			{
				do
				{
					arguments.push_back(parse_expression());
				} while (try_read_token(TOKEN::COMMA));
			}

			read_token(TOKEN::RPARENT, "expected ')' after parameter list in function call");

			return std::make_unique<ast::nodes::CallExpression>(std::move(identifier), std::move(arguments));
		}

		return std::make_unique<ast::nodes::VariableExpression>(std::move(identifier));
	}

	case TOKEN::LPARENT:
	{
		read_token();
		auto expression = parse_expression();
		read_token(TOKEN::RPARENT, "expected ')' after expression, following earlier '('");

		return expression;
	}

	default:
	{
		return nullptr;
	}
	}
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_unary()
{
	// TODO:
	// @Identifier
	// unary
	switch (m_current_token)
	{
	case TOKEN::KEYWORD_CONVERT:
	{
		return parse_type_cast();
	}

	default:
	{
		return parse_primary();
	}
	}
}

std::unique_ptr<ast::nodes::Expression>
Compiler::parse_binop_rhs(std::unique_ptr<ast::nodes::Expression> lhs, int current_priority)
{
	// inspired by llvm kaleidoscope binop parsing
	for (;;)
	{
		const auto first_op          = peek_binop();
		const auto first_op_priority = operator_priority(first_op);

		if (first_op == BinaryOperator::INVALID || first_op_priority < current_priority)
		{
			return lhs;
		}

		read_token();

		auto rhs = parse_unary();

		if (rhs == nullptr)
		{
			return nullptr;
		}

		const auto next_op          = peek_binop();
		const auto next_op_priority = operator_priority(next_op);

		if (next_op == BinaryOperator::INVALID || first_op_priority < next_op_priority)
		{
			rhs = parse_binop_rhs(std::move(rhs), first_op_priority + 1);

			if (rhs == nullptr)
			{
				return nullptr;
			}
		}

		// create expression
		lhs = std::make_unique<ast::nodes::BinaryExpression>(first_op, std::move(lhs), std::move(rhs));
	}
}

std::unique_ptr<ast::nodes::Expression> Compiler::parse_expression()
{
	auto lhs = parse_unary();

	return parse_binop_rhs(std::move(lhs));
}

std::unique_ptr<ast::nodes::Node> Compiler::parse_program()
{
	std::vector<std::unique_ptr<ast::nodes::Node>> nodes;

	for (;;)
	{
		switch (m_current_token)
		{
		case TOKEN::KEYWORD_VAR: nodes.push_back(parse_variable_declaration_block()); break;
		// case TOKEN::KEYWORD_TYPE: nodes.push_back(parse_type_definition()); break;
		case TOKEN::KEYWORD_FFI: nodes.push_back(parse_foreign_function_declaration()); break;
		case TOKEN::KEYWORD_INCLUDE: nodes.push_back(parse_include()); break;
		case TOKEN::KEYWORD_BEGIN: nodes.push_back(parse_block_statement()); break;
		case TOKEN::DOT: return std::make_unique<ast::nodes::Program>(std::move(nodes));
		default: error("expected declaration or block statement");
		}
	}
}

string_view Compiler::current_file() const { return m_file_name_stack.top(); }

void Compiler::show_source_context() const
{
	fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::white), "{}:{}: ", current_file().str(), m_lexer->lineno());
};

void Compiler::error(string_view error_message) const
{
	show_source_context();
	fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::red), "error: ");
	fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::white), "{}\n", error_message.str());

	note(fmt::format("while reading token '{}'", token_text().str()));

	throw CompilerError{"aborting due to past error"};
}

void Compiler::note(string_view note_message) const
{
	show_source_context();
	fmt::print(stderr, fmt::emphasis::bold | fg(fmt::color::green_yellow), "note:  ");
	fmt::print(stderr, "{}\n", note_message.str());
}

void Compiler::bug(string_view error_message) const
{
	show_source_context();
	fmt::print(stderr, fg(fmt::color::red), "error: COMPILER BUG!\n");

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

string_view Compiler::token_text() const { return m_lexer->YYText(); }

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

TOKEN Compiler::read_token() { return (m_current_token = TOKEN(m_lexer->yylex())); }
