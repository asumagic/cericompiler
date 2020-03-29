#pragma once

#include "nodes/forward.hpp"

namespace ast
{
class Visitor
{
	public:
	virtual ~Visitor() = default;

	virtual void operator()(nodes::BinaryExpression& expression)           = 0;
	virtual void operator()(nodes::UnaryExpression& expression)            = 0;
	virtual void operator()(nodes::VariableExpression& expression)         = 0;
	virtual void operator()(nodes::CallExpression& expression)             = 0;
	virtual void operator()(nodes::TypeCastExpression& expression)         = 0;
	virtual void operator()(nodes::StringLiteral& expression)              = 0;
	virtual void operator()(nodes::CharacterLiteral& expression)           = 0;
	virtual void operator()(nodes::IntegerLiteral& expression)             = 0;
	virtual void operator()(nodes::FloatLiteral& expression)               = 0;
	virtual void operator()(nodes::IfStatement& expression)                = 0;
	virtual void operator()(nodes::WhileStatement& expression)             = 0;
	virtual void operator()(nodes::ForStatement& expression)               = 0;
	virtual void operator()(nodes::BlockStatement& expression)             = 0;
	virtual void operator()(nodes::DisplayStatement& expression)           = 0;
	virtual void operator()(nodes::AssignmentStatement& expression)        = 0;
	virtual void operator()(nodes::VariableDeclarationBlock& expression)   = 0;
	virtual void operator()(nodes::ForeignFunctionDeclaration& expression) = 0;
	virtual void operator()(nodes::Include& expression)                    = 0;
	virtual void operator()(nodes::BuiltinType& expression)                = 0;
	virtual void operator()(nodes::UserType& expression)                   = 0;
	virtual void operator()(nodes::Program& expression)                    = 0;
};
} // namespace ast
