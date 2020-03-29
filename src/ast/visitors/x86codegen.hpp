#pragma once

#include "../nodes/all.hpp"
#include "../visitor.hpp"

namespace ast
{
namespace visitors
{
class CodeGenX86 : public VisitorWithErrorDefaults
{
	public:
	struct Context
	{};

	virtual void operator()(nodes::BinaryExpression& expression) override;
	/*virtual void operator()(nodes::UnaryExpression& expression) override;
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
	virtual void operator()(nodes::Include& expression) override;
	virtual void operator()(nodes::BuiltinType& expression) override;
	virtual void operator()(nodes::UserType& expression) override;
	 virtual void operator()(nodes::Program& expression) override;*/

	private:
	std::ostream& m_output_stream;
};
} // namespace visitors
} // namespace ast
