#include "typeevaluator.hpp"

#include "../nodes/all.hpp"
#include <fmt/core.h>

namespace ast
{
namespace visitors
{
void TypeEvaluator::operator()(nodes::Program& expression)
{
	for (auto& node : expression.nodes)
	{
		node->visit(*this);
	}
}

void TypeEvaluator::operator()(nodes::VariableExpression& expression)
{
	m_last_type = m_variable_types.at(expression.name);
}

void TypeEvaluator::operator()(nodes::IntegerLiteral& expression)
{
	// TODO: FUCK
}

void TypeEvaluator::operator()(nodes::BlockStatement& expression)
{
	for (auto& statement : expression.statements)
	{
		statement->visit(*this);
	}
}

void TypeEvaluator::operator()(nodes::DisplayStatement& expression)
{
	// TODO: check if normal type
}

bool TypeEvaluator::type_match(nodes::TypeName& a, nodes::TypeName& b) const { return false; }

std::string TypeEvaluator::type_name(nodes::TypeName& name) const { return "<todo>"; }

void TypeEvaluator::operator()(nodes::AssignmentStatement& expression)
{
	expression.lhs->visit(*this);
	auto& lhs_type = *m_last_type;

	expression.rhs->visit(*this);
	auto& rhs_type = *m_last_type;

	if (!type_match(lhs_type, rhs_type))
	{
		m_compiler.error(
			fmt::format("cannot assign value {} to variable with type {}", type_name(rhs_type), type_name(lhs_type)));
	}

	m_last_type = nullptr;
}

void TypeEvaluator::operator()(nodes::VariableDeclarationBlock& expression)
{
	for (auto& multiple_declaration : expression.multiple_declarations)
	{
		nodes::TypeName* type = multiple_declaration.type.get();

		for (const auto& name : multiple_declaration.names)
		{
			m_variable_types.emplace(name, type);
		}
	}
}

void TypeEvaluator::operator()([[maybe_unused]] nodes::ForeignFunctionDeclaration& expression)
{
	auto emplace_result = m_function_parameters.emplace(expression.name, std::vector<nodes::TypeName*>{});
	auto it             = emplace_result.first;

	for (const auto& parameter : expression.function.parameters)
	{
		it->second.push_back(parameter.get());
	}
}

void TypeEvaluator::operator()(nodes::Include& expression) { expression.contents->visit(*this); }

void TypeEvaluator::operator()(nodes::BuiltinType& expression) { m_last_type = &expression; }

void TypeEvaluator::operator()(nodes::UserType& expression) { m_last_type = &expression; }

} // namespace visitors
} // namespace ast
