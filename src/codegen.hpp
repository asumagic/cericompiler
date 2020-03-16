#pragma once

#include "util/string_view.hpp"

#include <cstdint>
#include <iosfwd>

class Variable;

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

	private:
	void alu_load_binop_i64();

	void alu_compare_i64(string_view instruction);

	std::size_t m_label_tag = {};

	std::ostream& m_output;
};
