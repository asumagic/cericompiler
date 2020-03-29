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
struct VariableDeclarationBlock : Node
{
	struct MultipleDeclaration
	{
		MultipleDeclaration(std::vector<std::string> names, std::unique_ptr<TypeName> type) :
			names{std::move(names)}, type{std::move(type)}
		{}

		std::vector<std::string>  names;
		std::unique_ptr<TypeName> type;
	};

	VariableDeclarationBlock(std::vector<MultipleDeclaration> multiple_declaration) :
		multiple_declarations{std::move(multiple_declaration)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::vector<MultipleDeclaration> multiple_declarations;
};

struct Function
{
	bool                                               variadic = false;
	std::unique_ptr<ast::nodes::TypeName>              return_type;
	std::vector<std::unique_ptr<ast::nodes::TypeName>> parameters;
	bool                                               foreign = false;
};

struct ForeignFunctionDeclaration : Node
{
	ForeignFunctionDeclaration(std::string name, Function function) :
		name{std::move(name)}, function{std::move(function)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::string name;
	Function    function;
};

struct Include : Node
{
	Include(std::string name, std::unique_ptr<Node> contents) : name{std::move(name)}, contents{std::move(contents)} {}

	virtual void visit(Visitor& visitor) override;

	std::string           name;
	std::unique_ptr<Node> contents;
};

struct Program : Node
{
	Program(std::vector<std::unique_ptr<Node>> nodes) : nodes{std::move(nodes)} {}

	virtual void visit(Visitor& visitor) override;

	std::vector<std::unique_ptr<Node>> nodes;
};

struct BuiltinType : TypeName
{
	BuiltinType(::Type type) : type{type} {}

	virtual void visit(Visitor& visitor) override;

	::Type type;
};

struct UserType : TypeName
{
	UserType(std::string name) : name{std::move(name)} {}

	virtual void visit(Visitor& visitor) override;

	std::string name;
};
} // namespace nodes
} // namespace ast
