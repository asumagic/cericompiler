#pragma once

#include "common.hpp"

#include <memory>
#include <vector>

namespace ast
{
namespace nodes
{
struct IfStatement : Statement
{
	IfStatement(
		std::unique_ptr<Expression> conditional,
		std::unique_ptr<Statement>  on_success,
		std::unique_ptr<Statement>  on_failure) :
		conditional{std::move(conditional)}, on_success{std::move(on_success)}, on_failure{std::move(on_failure)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::unique_ptr<Expression> conditional;
	std::unique_ptr<Statement>  on_success, on_failure;
};

struct WhileStatement : Statement
{
	WhileStatement(std::unique_ptr<Expression> conditional, std::unique_ptr<Statement> loop_body) :
		conditional{std::move(conditional)}, loop_body{std::move(loop_body)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::unique_ptr<Expression> conditional;
	std::unique_ptr<Statement>  loop_body;
};

struct ForStatement : Statement
{
	ForStatement(
		std::unique_ptr<Statement>  assignment,
		std::unique_ptr<Expression> stop_value,
		std::unique_ptr<Statement>  loop_body) :
		assignment{std::move(assignment)}, stop_value{std::move(stop_value)}, loop_body{std::move(loop_body)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::unique_ptr<Statement>  assignment;
	std::unique_ptr<Expression> stop_value;
	std::unique_ptr<Statement>  loop_body;
};

struct BlockStatement : Statement
{
	BlockStatement(std::vector<std::unique_ptr<Statement>> statements) : statements{std::move(statements)} {}

	virtual void visit(Visitor& visitor) override;

	std::vector<std::unique_ptr<Statement>> statements;
};

struct DisplayStatement : Statement
{
	DisplayStatement(std::unique_ptr<Expression> expression) : expression{std::move(expression)} {}

	virtual void visit(Visitor& visitor) override;

	std::unique_ptr<Expression> expression;
};

struct AssignmentStatement : Statement
{
	AssignmentStatement(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs) :
		lhs{std::move(lhs)}, rhs{std::move(rhs)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::unique_ptr<Expression> lhs, rhs;
};

} // namespace nodes
} // namespace ast
