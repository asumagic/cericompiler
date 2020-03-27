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
#include "exceptions.hpp"
#include "token.hpp"
#include "util/enums.hpp"
#include "util/string_view.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>
#include <vector>

bool operator==(const UserType& a, const UserType& b)
{
	if (a.category != b.category)
	{
		return false;
	}

	switch (a.category)
	{
	case UserType::Category::POINTER:
	{
		return a.layout_data.pointer.target == b.layout_data.pointer.target;
	}

	default:
	{
		throw UnimplementedTypeSupportError{};
	}
	}

	return true;
}

Compiler::Compiler(const CompilerConfig& config, string_view file_name, std::istream& input, std::ostream& output) :
	m_config{config},
	m_output_stream{output},
	m_lexer{new yyFlexLexer(input, output)},
	m_codegen{std::make_unique<CodeGen>(*this)}
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
		m_codegen->begin_program();

		read_token(); // Read first token
		parse_program();

		if (m_current_token != FEOF)
		{
			error(fmt::format("extraneous characters at end of file. did you use '.' instead of ';'?"));
		}

		m_codegen->finalize_program();
	}
	catch (const CompilerBug& e)
	{
		bug(e.what());
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

Type Compiler::parse_factor_identifier()
{
	const std::string name = token_text();

	read_token(); // Consume identifier

	if (m_current_token == TOKEN::LPARENT)
	{
		return parse_function_call_after_identifier(name, true);
	}
	else
	{
		return parse_variable_usage_after_identifier(name);
	}
}

void Compiler::parse_statement_identifier()
{
	const std::string name = token_text();
	read_token(); // Consume identifier

	if (m_current_token == TOKEN::LPARENT)
	{
		[[maybe_unused]] const Type type = parse_function_call_after_identifier(name);
	}
	else
	{
		parse_assignment_statement_after_identifier(name);
	}
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

Type Compiler::parse_variable_reference()
{
	read_token();

	expect_token(TOKEN::ID, "expected identifier after referencing operator '@'");
	const std::string name = token_text();
	read_token();

	const auto it = m_variables.find(name);
	if (it == m_variables.end())
	{
		error(fmt::format("use of undeclared identifier '{}'", name));
	}

	const VariableType& variable_type = it->second;

	UserType user_type(UserType::Category::POINTER);
	user_type.layout_data.pointer.target = variable_type.type;
	const Type pointer_type              = create_type(user_type);

	m_codegen->load_pointer_to_variable({it->first, variable_type});

	return pointer_type;
}

Type Compiler::parse_dereferencable()
{
	switch (m_current_token)
	{
	case LPARENT:
	{
		read_token();
		Type type = parse_expression();
		read_token(RPARENT, "expected ')'");

		return type;
	}

	case NOT:
	{
		read_token();
		Type type = parse_factor();
		check_type(type, Type::BOOLEAN);

		m_codegen->alu_not_bool();

		return Type::BOOLEAN;
	}

	case AT: return parse_variable_reference();
	case CHAR_LITERAL: return parse_character_literal();
	case INTEGER_LITERAL: return parse_integer_literal();
	case FLOAT_LITERAL: return parse_float_literal();
	case ID: return parse_factor_identifier();
	case KEYWORD_CONVERT: return parse_type_cast();
	default:
	{
		error("expected expression");
	}
	}
}

Type Compiler::parse_factor()
{
	Type current_type = parse_dereferencable();

	while (try_read_token(TOKEN::EXPONENT))
	{
		const auto it = m_user_types.find(current_type);

		if (it == m_user_types.end() || it->second.category != UserType::Category::POINTER)
		{
			error(fmt::format("cannot dereference non-pointer type '{}'", type_name(current_type).str()));
		}

		const UserType& type = it->second;

		m_codegen->load_value_from_pointer(type.layout_data.pointer.target);
		current_type = type.layout_data.pointer.target;
	}

	return current_type;
}

Type Compiler::parse_type_cast()
{
	read_token(); // CONVERT

	const Type source_type = parse_expression();

	read_token(KEYWORD_TO, "expected 'TO' after expression in CONVERT expression");

	const Type destination_type = parse_type();

	if (check_enum_range(source_type, Type::FIRST_USER_DEFINED, Type::LAST_USER_DEFINED)
		|| check_enum_range(destination_type, Type::FIRST_USER_DEFINED, Type::LAST_USER_DEFINED))
	{
		error(fmt::format(
			"incompatible types for explicit conversion {} -> {}",
			type_name(source_type).str(),
			type_name(destination_type).str()));
	}

	m_codegen->convert(source_type, destination_type);

	// right now just yolo it and don't convert
	return destination_type;
}

Type Compiler::parse_function_call_after_identifier(string_view name, bool expects_return)
{
	read_token(); // Consume '('

	const auto it = m_functions.find(name);
	if (it == m_functions.end())
	{
		error(fmt::format("use of undeclared function '{}'", name.str()));
	}

	const Function& function = it->second;

	if (function.return_type == Type::VOID && expects_return)
	{
		error(fmt::format("tried to get return value of function '{}' which does not return anything", name.str()));
	}

	if (function.variadic)
	{
		bug("variadic foreign function calls are not supported from the language for now");
	}

	FunctionCall call;
	call.function_name = name;
	call.return_type   = function.return_type;
	call.variadic      = function.variadic;

	m_codegen->function_call_prepare(call);

	// TODO: try catch to add context for the nth parameter and also for the function call
	std::size_t i = 0;
	if (m_current_token != TOKEN::RPARENT)
	{
		do
		{
			if (i >= function.parameters.size())
			{
				error(fmt::format(
					"too much parameters for function '{}', expected {}", name.str(), function.parameters.size()));
			}

			const FunctionParameter& declared_parameter = function.parameters[i];

			const Type expression_type = parse_expression();
			check_type(expression_type, declared_parameter.type);

			m_codegen->function_call_param(call, expression_type);

			++i;
		} while (try_read_token(TOKEN::COMMA));
	}

	if (i < function.parameters.size())
	{
		error(fmt::format(
			"not enough parameters for function '{}', expected {}", name.str(), function.parameters.size()));
	}

	m_codegen->function_call_finalize(call);

	read_token(TOKEN::RPARENT, "expected ')' after parameter list in function call");

	return function.return_type;
}

Type Compiler::parse_variable_usage_after_identifier(string_view name)
{
	const auto it = m_variables.find(name);
	if (it == m_variables.end())
	{
		error(fmt::format("use of undeclared identifier '{}'", name.str()));
	}

	const VariableType& type = it->second;

	m_codegen->load_variable({name, type});

	return type.type;
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
	for (;;)
	{
		switch (m_current_token)
		{
		case TOKEN::KEYWORD_VAR: parse_variable_declaration_block(); break;
		case TOKEN::KEYWORD_TYPE: parse_type_definition(); break;
		case TOKEN::KEYWORD_FFI: parse_foreign_function_declaration(); break;
		case TOKEN::KEYWORD_INCLUDE: parse_include(); break;
		default: return;
		}
	}
}

void Compiler::parse_variable_declaration_block()
{
	read_token(TOKEN::KEYWORD_VAR, "expected 'VAR' to begin variable declaration block");

	for (;;)
	{
		std::vector<std::string> current_declarations;

		do
		{
			current_declarations.push_back(token_text());
			read_token(); // Skip variable name
		} while (try_read_token(TOKEN::COMMA));

		read_token(COLON, "expected ':' after variable name list in declaration block");

		const Type type = parse_type();

		for (auto& name : current_declarations)
		{
			const auto emplace_result = m_variables.emplace(name, VariableType{type});
			const bool success        = emplace_result.second;

			if (!success)
			{
				error(fmt::format("duplicate declaration of variable '{}'", name));
			}
		}

		read_token(TOKEN::SEMICOLON, "expected ';' after variable declaration");

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
	};
}

void Compiler::parse_foreign_function_declaration()
{
	Function function;
	function.foreign = true;

	read_token(); // consume FFI

	expect_token(ID, "expected function name");
	const std::string name = token_text();
	read_token();

	read_token(LPARENT, "expected '(' after function name");

	if (m_current_token != RPARENT)
	{
		do
		{
			if (is_token_type(m_current_token))
			{
				const auto type = parse_type();
				function.parameters.push_back({type});
			}
		} while (try_read_token(COMMA));
	}

	read_token(RPARENT, "expected ')' after type list");

	read_token(COLON, "expected ':' after ')' to specify return type of foreign function");

	function.return_type = parse_type(true);

	read_token(SEMICOLON, "expected ';' after FFI declaration");

	const auto emplace_result = m_functions.emplace(name, std::move(function));
	const bool success        = emplace_result.second;

	if (!success)
	{
		error(fmt::format("duplicate declaration of function '{}'", name));
	}
}

void Compiler::parse_include()
{
	read_token(); // consume INCLUDE

	expect_token(TOKEN::STRINGCONST, "expected string literal after INCLUDE directive");
	const std::string path = token_text().substr(1, token_text().size() - 2);

	read_token(); // consume STRINGCONST include path

	read_token(SEMICOLON, "expected ';' after INCLUDE directive");

	const auto emplace_result = m_includes.emplace(path);
	const bool success        = emplace_result.second;

	if (!success)
	{
		// Already included this file
		return;
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

		parse_declaration_block();

		if (m_current_token != TOKEN::FEOF)
		{
			error("expected end of file");
		}
	}
	catch (const CompilerError& e)
	{
		restore_state();
		note("included here");
		throw;
	}

	// Restore old state
	restore_state();
}

Type Compiler::parse_type(bool allow_void)
{
	if (allow_void && try_read_token(VOID))
	{
		return Type::VOID;
	}

	if (is_token_type(m_current_token))
	{
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

	if (m_current_token == ID)
	{
		const auto it = m_typedefs.find(token_text());

		if (it == m_typedefs.end())
		{
			error("expected type but identifier does not refer to any type definition");
		}

		read_token();

		return it->second;
	}

	if (try_read_token(EXPONENT))
	{
		UserType type(UserType::Category::POINTER);
		type.layout_data.pointer.target = parse_type(false);

		return create_type(type);
	}

	error("expected type");
}

void Compiler::parse_type_definition()
{
	read_token(); // consume TYPE keyword

	expect_token(TOKEN::ID, "expected identifier after TYPE declaration");
	const std::string alias = token_text();
	read_token(); // consume identifier

	read_token(TOKEN::EQUAL, "expected '=' after aliased name in TYPE declaration");

	const Type aliased = parse_type();

	read_token(TOKEN::SEMICOLON, "expected ';' after TYPE declaration");

	const auto emplace_result = m_typedefs.emplace(alias, aliased);
	const bool success        = emplace_result.second;

	if (!success)
	{
		error(fmt::format("duplicate declaration of type '{}'", alias));
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
	read_token(); // We needed the token_text up until now - consume the identifier

	return parse_assignment_statement_after_identifier(name);
}

Variable Compiler::parse_assignment_statement_after_identifier(string_view name)
{
	const auto it = m_variables.find(name);

	if (it == m_variables.end())
	{
		error(fmt::format("assignment of undeclared variable '{}'", name.str()));
	}

	const VariableType& variable_type = it->second;

	if (m_current_token == TOKEN::EXPONENT)
	{
		Type current_type = variable_type.type;

		std::vector<Type> dereference_stack;

		while (try_read_token(TOKEN::EXPONENT))
		{
			// TODO: deduplicate code with parse_factor()
			const auto it = m_user_types.find(current_type);

			if (it == m_user_types.end() || it->second.category != UserType::Category::POINTER)
			{
				error(fmt::format("cannot dereference non-pointer type '{}'", type_name(current_type).str()));
			}

			const UserType& type = it->second;

			dereference_stack.push_back(type.layout_data.pointer.target);
			current_type = type.layout_data.pointer.target;
		}

		// TODO: deduplicate code with below
		read_token(ASSIGN, "expected ':=' in variable assignment");

		Type type = parse_expression();

		m_codegen->load_variable({name, variable_type});

		dereference_stack.resize(dereference_stack.size() - 1); // ignore the last one
		for (const Type type : dereference_stack)
		{
			m_codegen->load_value_from_pointer(type);
		}

		m_codegen->store_value_to_pointer(type);

		check_type(type, current_type);

		return {};
	}

	read_token(ASSIGN, "expected ':=' in variable assignment");

	Type type = parse_expression();

	m_codegen->store_variable({name, variable_type});

	check_type(type, variable_type.type);

	return {name, variable_type};
}

void Compiler::parse_if_statement()
{
	IfStatement if_statement;
	m_codegen->statement_if_prepare(if_statement);

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
	WhileStatement while_statement;
	m_codegen->statement_while_prepare(while_statement);

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

	ForStatement for_statement;
	m_codegen->statement_for_prepare(for_statement, assignment);
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
	read_token(KEYWORD_BEGIN, "expected 'BEGIN' to begin block statement");

	// Check for an empty block statement, e.g. BEGIN END
	if (try_read_token(KEYWORD_END))
	{
		return;
	}

	do
	{
		parse_statement();
	} while (try_read_token(TOKEN::SEMICOLON));

	read_token(KEYWORD_END, "expected 'END' to finish block statement");
}

void Compiler::parse_display_statement()
{
	read_token();
	const Type type = parse_expression();

	// check if user defined, if not its not allowed
	if (check_enum_range(type, Type::FIRST_USER_DEFINED, Type::LAST_USER_DEFINED))
	{
		error(fmt::format("DISPLAY is not supported for type {}", type_name(type).str()));
	}

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
	case TOKEN::ID: parse_statement_identifier(); break;
	default: error("expected statement");
	}
}

void Compiler::parse_main_block_statement()
{
	m_codegen->begin_main_procedure();

	parse_block_statement();
	read_token(DOT, "expected '.' at end of program");

	m_codegen->finalize_main_procedure();
}

void Compiler::parse_program()
{
	m_codegen->begin_executable_section();
	parse_declaration_block();
	parse_main_block_statement();
	m_codegen->finalize_executable_section();

	m_codegen->begin_global_data_section();
	emit_global_variables();
	m_codegen->finalize_global_data_section();
}

Type Compiler::create_type(UserType user_type)
{
	// TODO: have another map for the opposite lookup
	for (const auto it : m_user_types)
	{
		const Type      other_type      = it.first;
		const UserType& other_user_type = it.second;

		if (user_type == other_user_type)
		{
			return other_type;
		}
	}

	const auto emplace_result = m_user_types.emplace(allocate_type_id(), user_type);
	const auto it             = emplace_result.first;
	const bool success        = emplace_result.second;

	if (!success)
	{
		bug("type id was already allocated");
	}

	return it->first;
}

Type Compiler::allocate_type_id()
{
	m_first_free_type = Type(underlying_cast(m_first_free_type) + 1);
	return m_first_free_type;
}

void Compiler::emit_global_variables()
{
	for (const auto& it : m_variables)
	{
		const auto&         name = it.first;
		const VariableType& type = it.second;

		m_codegen->define_global_variable({name, type});
	}
}

string_view Compiler::current_file() const { return m_file_name_stack.top(); }

std::string Compiler::source_context() const
{
	return fmt::format("{}:{}: ", current_file().str(), m_lexer->lineno());
};

void Compiler::error(string_view error_message) const
{
	fmt::print(stderr, fg(fmt::color::red), "{}error: {}\n", source_context(), error_message.str());
	note(fmt::format("while reading token '{}'", token_text().str()));

	throw CompilerError{"aborting due to past error"};
}

void Compiler::note(string_view note_message) const
{
	fmt::print(stderr, fg(fmt::color::gray), "{}note:  {}\n", source_context(), note_message.str());
}

void Compiler::bug(string_view error_message) const
{
	fmt::print(stderr, fg(fmt::color::red), "{}error: COMPILER BUG!\n", source_context());

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
