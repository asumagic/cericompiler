#pragma once

#include "../../compiler.hpp"
#include "../visitor.hpp"

#include <map>
#include <vector>

namespace ast
{
namespace visitors
{
//! \brief AST walker that evaluates and sets the baked type of every expression and performs type checking.
//! \details Type checking is performed over all expressions or e.g. function calls.
class TypeEvaluator : public VisitorWithErrorDefaults
{
	public:
	TypeEvaluator(Compiler& compiler) : m_compiler{compiler} {}

	//	virtual void operator()(nodes::BinaryExpression& expression) override;
	//	virtual void operator()(nodes::UnaryExpression& expression) override;
	virtual void operator()(nodes::VariableExpression& expression) override;
	//	virtual void operator()(nodes::CallExpression& expression) override;
	//	virtual void operator()(nodes::TypeCastExpression& expression) override;
	//	virtual void operator()(nodes::StringLiteral& expression) override;
	//	virtual void operator()(nodes::CharacterLiteral& expression) override;
	virtual void operator()(nodes::IntegerLiteral& expression) override;
	//	virtual void operator()(nodes::FloatLiteral& expression) override;
	//	virtual void operator()(nodes::IfStatement& expression) override;
	//	virtual void operator()(nodes::WhileStatement& expression) override;
	//	virtual void operator()(nodes::ForStatement& expression) override;
	virtual void operator()(nodes::BlockStatement& expression) override;
	virtual void operator()(nodes::DisplayStatement& expression) override;
	virtual void operator()(nodes::AssignmentStatement& expression) override;
	virtual void operator()(nodes::VariableDeclarationBlock& expression) override;
	virtual void operator()(nodes::ForeignFunctionDeclaration& expression) override;
	virtual void operator()(nodes::Include& expression) override;
	virtual void operator()(nodes::BuiltinType& expression) override;
	virtual void operator()(nodes::UserType& expression) override;
	virtual void operator()(nodes::Program& expression) override;

	private:
	bool        type_match(nodes::TypeName& a, nodes::TypeName& b) const;
	std::string type_name(nodes::TypeName& name) const;

	Compiler& m_compiler;

	nodes::TypeName* m_last_type = nullptr;

	std::map<std::string, nodes::TypeName*>              m_variable_types;
	std::map<std::string, std::vector<nodes::TypeName*>> m_function_parameters;
};
} // namespace visitors
} // namespace ast
