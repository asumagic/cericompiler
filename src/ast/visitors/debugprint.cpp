#include "debugprint.hpp"

#include "../../util/enums.hpp"
#include "../nodes/all.hpp"

#include <fmt/core.h>

namespace ast
{
namespace visitors
{
void DebugPrint::operator()(nodes::BinaryExpression& expression)
{
	print_indented("BinaryExpression (op={})", underlying_cast(expression.op));

	indented("operands", [&] {
		expression.lhs->visit(*this);
		expression.rhs->visit(*this);
	});
}

void DebugPrint::operator()(nodes::UnaryExpression& expression)
{
	print_indented("UnaryExpression (op={})", underlying_cast(expression.op));

	indented("operand", [&] { expression.expression->visit(*this); });
}

void DebugPrint::operator()(nodes::VariableExpression& expression)
{
	print_indented("VariableExpression '{}'", expression.name);
}

void DebugPrint::operator()(nodes::CallExpression& expression)
{
	print_indented("CallExpression '{}'", expression.function_name);

	indented("arguments", [&] {
		for (auto& argument : expression.arguments)
		{
			argument->visit(*this);
		}
	});
}

void DebugPrint::operator()(nodes::TypeCastExpression& expression)
{
	print_indented("TypeCastExpression");

	indented("targettype", [&] { expression.target_type->visit(*this); });
	indented("operand", [&] { expression.expression->visit(*this); });
}

void DebugPrint::operator()(nodes::StringLiteral& expression)
{
	print_indented("StringLiteral '{}'", expression.value);
}
void DebugPrint::operator()(nodes::CharacterLiteral& expression)
{
	print_indented("CharLiteral '{}'", expression.value);
}
void DebugPrint::operator()(nodes::IntegerLiteral& expression)
{
	print_indented("IntegerLiteral '{}'", expression.value);
}
void DebugPrint::operator()(nodes::FloatLiteral& expression) { print_indented("FloatLiteral '{}'", expression.value); }

void DebugPrint::operator()(nodes::IfStatement& expression)
{
	print_indented("IfStatement");
	indented("conditional", [&] { expression.conditional->visit(*this); });
	indented("branchtrue", [&] { expression.on_success->visit(*this); });
	indented("branchfalse", [&] { expression.on_failure->visit(*this); });
}

void DebugPrint::operator()(nodes::WhileStatement& expression)
{
	print_indented("WhileStatement");
	indented("conditional", [&] { expression.conditional->visit(*this); });
	indented("loopbody", [&] { expression.loop_body->visit(*this); });
}

void DebugPrint::operator()(nodes::ForStatement& expression)
{
	print_indented("ForStatement");
	indented("assignment", [&] { expression.assignment->visit(*this); });
	indented("stopvalue", [&] { expression.stop_value->visit(*this); });
	indented("loopbody", [&] { expression.loop_body->visit(*this); });
}

void DebugPrint::operator()(nodes::BlockStatement& expression)
{
	print_indented("BlockStatement");
	indented("statements", [&] {
		for (auto& statement : expression.statements)
		{
			statement->visit(*this);
		}
	});
}

void DebugPrint::operator()(nodes::DisplayStatement& expression)
{
	print_indented("DisplayStatement");
	indented("expression", [&] { expression.expression->visit(*this); });
}

void DebugPrint::operator()(nodes::AssignmentStatement& expression)
{
	print_indented("AssignmentStatement");
	indented("assigned", [&] { expression.lhs->visit(*this); });
	indented("value", [&] { expression.rhs->visit(*this); });
}

void DebugPrint::operator()(nodes::VariableDeclarationBlock& expression)
{
	print_indented("VariableDeclarationBlock");
	indented("multipledeclarations", [&] {
		for (const auto& multiple_declaration : expression.multiple_declarations)
		{
			print_indented("VariableMultipleDeclaration");
			indented("type", [&] { multiple_declaration.type->visit(*this); });
			indented("names", [&] {
				for (const auto& name : multiple_declaration.names)
				{
					print_indented("'{}'", name);
				}
			});
		}
	});
}

void DebugPrint::operator()(nodes::ForeignFunctionDeclaration& expression)
{
	print_indented("ForeignFunctionDeclaration '{}' (types=todo)", expression.name);
}

void DebugPrint::operator()(nodes::Include& expression)
{
	print_indented("Include '{}'", expression.name);
	indented("contents", [&] { expression.contents->visit(*this); });
}

void DebugPrint::operator()(nodes::BuiltinType& expression)
{
	print_indented("Builtin type '{}'", type_name(expression.type).str());
}

void DebugPrint::operator()(nodes::UserType& expression) { print_indented("User type '{}'", expression.name); }

void DebugPrint::operator()(nodes::Program& expression)
{
	print_indented("Program");
	indented("nodes", [&] {
		for (auto& node : expression.nodes)
		{
			node->visit(*this);
		}
	});
}

template<class... Ts>
void DebugPrint::print_indented(Ts&&... params)
{
	fmt::print(stderr, "{}{}\n", std::string(m_depth * 4, ' '), fmt::format(std::forward<Ts>(params)...));
}

template<class Func>
void DebugPrint::indented(const std::string& category, Func&& func)
{
	print_indented("...{}:", category);

	++m_depth;
	func();
	--m_depth;
}
} // namespace visitors
} // namespace ast
