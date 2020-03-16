#pragma once

#include <iosfwd>

class CodeGen
{
	public:
	CodeGen(std::ostream& output) : m_output{output} {}

	void begin_executable();
	void finalize_executable();

	void begin_main_procedure();
	void finalize_main_procedure();

	void begin_global_data();
	void finalize_global_data();

	private:
	std::ostream& m_output;
};
