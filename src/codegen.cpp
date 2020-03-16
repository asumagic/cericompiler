#include "codegen.hpp"

#include <ostream>

void CodeGen::begin_executable() { m_output << ".text\n"; }
void CodeGen::finalize_executable() {}

void CodeGen::begin_main_procedure()
{
	m_output << ".globl main\n"
				"main:\n"
				"\t# Save the position of the top of the stack\n"
				"\tmovq %rsp, %rbp\n";
}

void CodeGen::finalize_main_procedure() {}

void CodeGen::begin_global_data()
{
	m_output << ".data\n"
				".align 8\n"
				"__cc_format_string_llu: .string \"%llu\\n\"\n";
}

void CodeGen::finalize_global_data() {}
