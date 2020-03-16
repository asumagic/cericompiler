#pragma once

#include "types.hpp"
#include "util/string_view.hpp"

#include <cstdint>
#include <iosfwd>

class Variable;

struct IfStatement
{
	std::size_t saved_tag;
};

struct WhileStatement
{
	std::size_t saved_tag;
};

struct ForStatement
{
	std::size_t     saved_tag;
	const Variable* variable;
};

class CodeGen
{
	public:
	CodeGen(std::ostream& output) : m_output{output} {}

	void begin_program();
	void finalize_program();

	void begin_executable_section();
	void finalize_executable_section();

	void begin_main_procedure();
	void finalize_main_procedure();

	void begin_global_data_section();
	void finalize_global_data_section();

	void define_global_variable(const Variable& variable);

	void load_variable(const Variable& variable);
	void load_i64(std::int64_t value);

	void store_variable(const Variable& variable);

	void alu_and_bool();
	void alu_or_bool();

	void alu_add_i64();
	void alu_sub_i64();
	void alu_multiply_i64();
	void alu_divide_i64();
	void alu_modulus_i64();

	void alu_equal_i64();
	void alu_not_equal_i64();
	void alu_greater_equal_i64();
	void alu_lower_equal_i64();
	void alu_greater_i64();
	void alu_lower_i64();

	// HACK: using the boolean return value to return whether the conversion was supported or not
	bool convert(Type source, Type destination);

	IfStatement statement_if_prepare();
	void        statement_if_post_check(IfStatement);
	void        statement_if_with_else(IfStatement);
	void        statement_if_without_else(IfStatement);
	void        statement_if_finalize(IfStatement);

	WhileStatement statement_while_prepare();
	void           statement_while_post_check(WhileStatement);
	void           statement_while_finalize(WhileStatement);

	ForStatement statement_for_prepare(const Variable& assignement_variable);
	void         statement_for_post_assignment(ForStatement);
	void         statement_for_post_check(ForStatement);
	void         statement_for_finalize(ForStatement);

	void debug_display_i64();
	void debug_display_boolean();
	void debug_display_char();

	private:
	void alu_load_binop_i64();

	void alu_compare_i64(string_view instruction);

	void debug_call_printf();

	std::size_t m_label_tag = 0;

	std::ostream& m_output;
};
