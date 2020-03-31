#include "x86codegen.hpp"

#include "../nodes/all.hpp"

#include <algorithm>
#include <fmt/core.h>

namespace ast
{
namespace visitors
{
namespace x86
{
std::string Variable::mangled_name() const { return fmt::format("__ceri_var_{}", name); }

void CodeGen::operator()(nodes::BlockStatement& expression)
{
	m_output_stream << fmt::format(
		".globl {mainfunc}\n"
		"{mainfunc}:\n"
		"\tmovq %rsp, %rbp # save the position of the top of the stack\n",
		fmt::arg("mainfunc", mangle_name("main")));

	StatementEvaluator evaluator{*this};
	for (auto& statement : expression.statements)
	{
		statement->visit(evaluator);
	}

	m_output_stream << "\tmovq %rbp, %rsp # restore the position of the top of the stack\n"
					   "\tmovq $0, %rax # set exit code\n"
					   "\tret\n";
}

void CodeGen::operator()(nodes::VariableDeclarationBlock& expression)
{
	for (auto& multiple_declaration : expression.multiple_declarations)
	{
		nodes::TypeName* type = multiple_declaration.type.get();

		for (const auto& name : multiple_declaration.names)
		{
			m_variables.push_back({name, type});
		}
	}
}

void CodeGen::operator()(nodes::ForeignFunctionDeclaration& expression) { m_ffi.push_back(&expression); }

void CodeGen::operator()(nodes::Include& expression) { expression.contents->visit(*this); }

void CodeGen::operator()(nodes::Program& expression)
{
	for (auto& node : expression.nodes)
	{
		node->visit(*this);
	}
}

void CodeGen::bug(string_view message) const
{
	// TODO: invoke compiler.bug instead
	fmt::print(stderr, "x86 codegen bug: {}\n", message.str());
	std::exit(2);
}

std::string CodeGen::mangle_name(string_view name) const
{
	// TODO: target support again
	return name;
}

Variable& CodeGen::get_variable(string_view name)
{
	return *std::find_if(
		m_variables.begin(), m_variables.end(), [&](const Variable& variable) { return variable.name == name; });
}

void ExpressionEvaluator::operator()(nodes::BinaryExpression& expression)
{
	expression.lhs->visit(*this);
	expression.rhs->visit(*this);

	auto* type = static_cast<nodes::BuiltinType*>(expression.baked_type.get());

	const auto unsupported
		= [this] { m_codegen.bug("unsupported type or operation during evaluation of binary expression"); };

	switch (type->type)
	{
	case Type::UNSIGNED_INT:
	case Type::BOOLEAN:
	{
		m_codegen.m_output_stream << "\tpopq %rbx\n"
									 "\tpopq %rax\n";
		break;
	}

	case Type::DOUBLE:
	{
		m_codegen.m_output_stream << "\tfldl 8(%rsp)\n"
									 "\taddq $16, %rsp\n";
		break;
	}

	default: unsupported();
	}

	using BinOp = nodes::BinaryExpression::Operator;

	const auto emit_store_f64 = [&] {
		m_codegen.m_output_stream << "\tsubq $8, %rsp\n"
									 "fstpl (%rsp)\n";
	};

	const auto emit_push_true_on_branch = [&](const char* op) {
		switch (type->type)
		{
		case Type::UNSIGNED_INT:
		{
			m_codegen.m_output_stream << "\tcmpq %rbx, %rax\n";
			break;
		}

		case Type::DOUBLE:
		{
			m_codegen.m_output_stream << "\tfcomip\n"
										 "\tfstp %st(0) # clear fp stack\n";
			break;
		}

		default: unsupported();
		}

		++m_codegen.m_label_tag;

		m_codegen.m_output_stream << fmt::format(
			"\t{jumpinstruction} __true{tag}\n"
			"\tpushq $0x0 # No branching: push false\n"
			"\tjmp __next{tag}\n"
			"__true{tag}:\n"
			"\tpushq $0xFFFFFFFFFFFFFFFF\n"
			"__next{tag}:\n",
			fmt::arg("jumpinstruction", op),
			fmt::arg("tag", m_codegen.m_label_tag));
	};

	switch (expression.op)
	{
	case BinOp::ADD:
	{
		switch (type->type)
		{
		case Type::UNSIGNED_INT:
		{
			m_codegen.m_output_stream << "\taddq %rbx, %rax\n"
										 "\tpushq %rax\n";
			break;
		}

		case Type::DOUBLE:
		{
			m_codegen.m_output_stream << "\tfaddp %st(0), %st(1)\n";
			emit_store_f64();
			break;
		}

		default: unsupported();
		}

		break;
	}

	case BinOp::SUBTRACT:
	{
		switch (type->type)
		{
		case Type::UNSIGNED_INT:
		{
			m_codegen.m_output_stream << "\tsubq %rbx, %rax\n"
										 "\tpushq %rax\n";
			break;
		}

		case Type::DOUBLE:
		{
			m_codegen.m_output_stream << "\tfsubp %st(0), %st(1)\n";
			emit_store_f64();
			break;
		}

		default: unsupported();
		}

		break;
	}

	case BinOp::MULTIPLY:
	{
		switch (type->type)
		{
		case Type::UNSIGNED_INT:
		{
			m_codegen.m_output_stream << "\tmulq %rbx, %rax\n"
										 "\tpushq %rax\n";
			break;
		}

		case Type::DOUBLE:
		{
			m_codegen.m_output_stream << "\tfmulp %st(0), %st(1)\n";
			emit_store_f64();
			break;
		}

		default: unsupported();
		}

		break;
	}

	case BinOp::DIVIDE:
	{
		switch (type->type)
		{
		case Type::UNSIGNED_INT:
		{
			m_codegen.m_output_stream << "\tmovq $0, %rdx # higher part of numerator\n"
										 "\tdiv %rbx # quotient goes to %rax\n"
										 "\tpushq %rax\n";
			break;
		}

		case Type::DOUBLE:
		{
			m_codegen.m_output_stream << "\tfdivp %st(0), %st(1)\n";
			emit_store_f64();
			break;
		}

		default: unsupported();
		}

		break;
	}

	case BinOp::MODULUS:
	{
		switch (type->type)
		{
		case Type::UNSIGNED_INT:
		{
			m_codegen.m_output_stream << "\tmovq $0, %rdx # higher part of numerator\n"
										 "\tdiv %rbx # remainder goes to %rdx\n"
										 "\tpushq %rdx\n";
			break;
		}

		case Type::DOUBLE:
		{
			m_codegen.m_output_stream << "\tfdivp %st(0), %st(1)\n";
			emit_store_f64();
			break;
		}

		default: unsupported();
		}

		break;
	}

	case BinOp::EQUAL: emit_push_true_on_branch("je"); break;
	case BinOp::NOT_EQUAL: emit_push_true_on_branch("jne"); break;
	case BinOp::GREATER_EQUAL: emit_push_true_on_branch("jae"); break;
	case BinOp::LOWER_EQUAL: emit_push_true_on_branch("jbe"); break;
	case BinOp::GREATER: emit_push_true_on_branch("ja"); break;
	case BinOp::LOWER: emit_push_true_on_branch("jb"); break;

	case BinOp::LOGICAL_AND:
	{
		m_codegen.m_output_stream << "\tandq %rax, %rbx\n"
									 "\tpushq %rax\n";

		break;
	}

	case BinOp::LOGICAL_OR:
	{
		m_codegen.m_output_stream << "\torq %rax, %rbx\n"
									 "\tpushq %rax\n";

		break;
	}

	default: unsupported();
	}
}

void ExpressionEvaluator::operator()(nodes::VariableExpression& expression)
{
	m_codegen.m_output_stream << fmt::format(
		"\tpushq {}(%rip)\n", m_codegen.get_variable(expression.name).mangled_name());
}

void StatementEvaluator::operator()(nodes::AssignmentStatement& expression)
{
	// TODO: store value to pointer
	auto* lhs = dynamic_cast<nodes::VariableExpression*>(expression.lhs.get());

	m_codegen.m_output_stream << fmt::format("\tpopq {}(%rip)\n", m_codegen.get_variable(lhs->name).mangled_name());
}

} // namespace x86
} // namespace visitors
} // namespace ast
