#pragma once

#include "../nodes/all.hpp"
#include "../visitor.hpp"

#include <fmt/core.h>

namespace ast
{
namespace visitors
{
class DebugPrint : public Visitor
{
	public:
	virtual void operator()(nodes::BinaryExpression& expression) override;
	virtual void operator()(nodes::UnaryExpression& expression) override;
	virtual void operator()(nodes::VariableExpression& expression) override;
	virtual void operator()(nodes::CallExpression& expression) override;
	virtual void operator()(nodes::TypeCastExpression& expression) override;
	virtual void operator()(nodes::StringLiteral& expression) override;
	virtual void operator()(nodes::CharacterLiteral& expression) override;
	virtual void operator()(nodes::IntegerLiteral& expression) override;
	virtual void operator()(nodes::FloatLiteral& expression) override;
	virtual void operator()(nodes::IfStatement& expression) override;
	virtual void operator()(nodes::WhileStatement& expression) override;
	virtual void operator()(nodes::ForStatement& expression) override;
	virtual void operator()(nodes::BlockStatement& expression) override;
	virtual void operator()(nodes::DisplayStatement& expression) override;
	virtual void operator()(nodes::AssignmentStatement& expression) override;
	virtual void operator()(nodes::VariableDeclarationBlock& expression) override;
	virtual void operator()(nodes::ForeignFunctionDeclaration& expression) override;
	virtual void operator()(nodes::Program& expression) override;

	private:
	template<class... Ts>
	void print_indented(Ts&&... params);

	template<class Func>
	void indented(const std::string& category, Func&& func);

	int m_depth = 0;
};
} // namespace visitors
} // namespace ast
