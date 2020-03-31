#pragma once

#include "../../util/string_view.hpp"
#include "../visitor.hpp"

#include <vector>

namespace ast
{
namespace visitors
{
namespace x86
{
struct Variable
{
	std::string      name;
	nodes::TypeName* type;

	std::string mangled_name() const;
};

class CodeGen : public VisitorWithErrorDefaults
{
	friend class StatementEvaluator;
	friend class ExpressionEvaluator;

	public:
	CodeGen(std::ostream& output_stream) : m_output_stream{output_stream} {}

	// todo somewhere most likely
	//	virtual void operator()(nodes::BuiltinType& expression) override;
	//	virtual void operator()(nodes::UserType& expression) override;

	virtual void operator()(nodes::BlockStatement& expression) override;
	virtual void operator()(nodes::VariableDeclarationBlock& expression) override;
	virtual void operator()(nodes::ForeignFunctionDeclaration& expression) override;
	virtual void operator()(nodes::Include& expression) override;
	virtual void operator()(nodes::Program& expression) override;

	private:
	[[noreturn]] void bug(string_view message) const;

	std::string mangle_name(string_view name) const;

	Variable& get_variable(string_view name);

	std::vector<nodes::ForeignFunctionDeclaration*> m_ffi; // TODO: probably shouldn't do that
	std::vector<Variable>                           m_variables;

	std::ostream& m_output_stream;
	std::size_t   m_label_tag = 0;
};

class StatementEvaluator : public VisitorWithErrorDefaults
{
	public:
	StatementEvaluator(CodeGen& codegen) : m_codegen{codegen} {}

	/*virtual void operator()(nodes::IfStatement& expression) override;
	virtual void operator()(nodes::WhileStatement& expression) override;
	virtual void operator()(nodes::ForStatement& expression) override;
	virtual void operator()(nodes::BlockStatement& expression) override;
	virtual void operator()(nodes::DisplayStatement& expression) override;*/
	virtual void operator()(nodes::AssignmentStatement& expression) override;

	private:
	CodeGen m_codegen;
};

class ExpressionEvaluator : public VisitorWithErrorDefaults
{
	public:
	ExpressionEvaluator(CodeGen& codegen) : m_codegen{codegen} {}

	virtual void operator()(nodes::BinaryExpression& expression) override;
	//	virtual void operator()(nodes::UnaryExpression& expression) override;
	virtual void operator()(nodes::VariableExpression& expression) override;
	//	virtual void operator()(nodes::CallExpression& expression) override;
	//	virtual void operator()(nodes::TypeCastExpression& expression) override;
	//	virtual void operator()(nodes::StringLiteral& expression) override;
	//	virtual void operator()(nodes::CharacterLiteral& expression) override;
	//	virtual void operator()(nodes::IntegerLiteral& expression) override;
	//	virtual void operator()(nodes::FloatLiteral& expression) override;

	private:
	CodeGen& m_codegen;
};
} // namespace x86

using CodeGenX86 = x86::CodeGen;
} // namespace visitors
} // namespace ast
