#include "visitor.hpp"

#include <fmt/core.h>

namespace ast
{
void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::BinaryExpression& expression)
{
	unexpected("BinaryExpression");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::UnaryExpression& expression)
{
	unexpected("UnaryExpression");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::VariableExpression& expression)
{
	unexpected("VariableExpression");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::CallExpression& expression)
{
	unexpected("CallExpression");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::TypeCastExpression& expression)
{
	unexpected("TypeCastExpression");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::StringLiteral& expression)
{
	unexpected("StringLiteral");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::CharacterLiteral& expression)
{
	unexpected("CharacterLiteral");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::IntegerLiteral& expression)
{
	unexpected("IntegerLiteral");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::FloatLiteral& expression)
{
	unexpected("FloatLiteral");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::IfStatement& expression)
{
	unexpected("IfStatement");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::WhileStatement& expression)
{
	unexpected("WhileStatement");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::ForStatement& expression)
{
	unexpected("ForStatement");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::BlockStatement& expression)
{
	unexpected("BlockStatement");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::DisplayStatement& expression)
{
	unexpected("DisplayStatement");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::AssignmentStatement& expression)
{
	unexpected("AssignmentStatement");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::VariableDeclarationBlock& expression)
{
	unexpected("VariableDeclarationBlock");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::ForeignFunctionDeclaration& expression)
{
	unexpected("ForeignFunctionDeclaration");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::Include& expression) { unexpected("Include"); }

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::BuiltinType& expression)
{
	unexpected("BuiltinType");
}

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::UserType& expression) { unexpected("UserType"); }

void VisitorWithErrorDefaults::operator()([[maybe_unused]] nodes::Program& expression) { unexpected("Program"); }

void VisitorWithErrorDefaults::unexpected(string_view message)
{
	// TODO: invoke compiler.bug instead
	fmt::print(stderr, "AST walker encountered unexpected node: {}\n", message.str());
	std::exit(2);
}

} // namespace ast
