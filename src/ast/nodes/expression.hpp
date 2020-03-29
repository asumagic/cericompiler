#pragma once

#include "../../types.hpp"
#include "common.hpp"

#include <memory>
#include <string>
#include <vector>

namespace ast
{
namespace nodes
{
struct BinaryExpression : Expression
{
	enum class Operator
	{
		INVALID,

		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		MODULUS,

		EQUAL,
		NOT_EQUAL,
		GREATER_EQUAL,
		LOWER_EQUAL,
		GREATER,
		LOWER,

		LOGICAL_AND,
		LOGICAL_OR
	};

	BinaryExpression(Operator op, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs) :
		op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)}
	{}

	virtual void visit(Visitor& visitor) override;

	Operator                    op;
	std::unique_ptr<Expression> lhs, rhs;
};

struct UnaryExpression : Expression
{
	enum class Operator
	{
		INVALID,

		REFERENCE,
		DEREFERENCE,

		MINUS,

		LOGICAL_NOT
	};

	UnaryExpression(Operator op, std::unique_ptr<Expression> expression) : op{op}, expression{std::move(expression)} {}

	virtual void visit(Visitor& visitor) override;

	Operator                    op;
	std::unique_ptr<Expression> expression;
};

struct VariableExpression : Expression
{
	VariableExpression(std::string name) : name{std::move(name)} {}

	virtual void visit(Visitor& visitor) override;

	std::string name;
};

struct CallExpression : Expression
{
	CallExpression(std::string function_name, std::vector<std::unique_ptr<Expression>> arguments) :
		function_name{std::move(function_name)}, arguments{std::move(arguments)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::string                              function_name;
	std::vector<std::unique_ptr<Expression>> arguments;
};

struct TypeCastExpression : Expression
{
	TypeCastExpression(Type target_type, std::unique_ptr<Expression> expression) :
		target_type{target_type}, expression{std::move(expression)}
	{}

	virtual void visit(Visitor& visitor) override;

	Type                        target_type;
	std::unique_ptr<Expression> expression;
};

struct StringLiteral : Expression
{
	StringLiteral(std::string value) : value{std::move(value)} {}

	virtual void visit(Visitor& visitor) override;

	std::string value;
};

struct CharacterLiteral : Expression
{
	CharacterLiteral(char value) : value{value} {}

	virtual void visit(Visitor& visitor) override;

	char value;
};

struct IntegerLiteral : Expression
{
	IntegerLiteral(std::uint64_t value) : value{value} {}

	virtual void visit(Visitor& visitor) override;

	std::uint64_t value;
};

struct FloatLiteral : Expression
{
	FloatLiteral(double value) : value{value} {}

	virtual void visit(Visitor& visitor) override;

	double value;
};
} // namespace nodes
} // namespace ast
