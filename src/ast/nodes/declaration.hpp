#pragma once

#include "../../function.hpp"
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
		MultipleDeclaration(std::vector<std::string> names, Type type) : names{std::move(names)}, type{type} {}

		std::vector<std::string> names;
		Type                     type;
	};

	VariableDeclarationBlock(std::vector<MultipleDeclaration> multiple_declaration) :
		multiple_declarations{std::move(multiple_declaration)}
	{}

	virtual void visit(Visitor& visitor) override;

	std::vector<MultipleDeclaration> multiple_declarations;
};

struct ForeignFunctionDeclaration : Node
{
	ForeignFunctionDeclaration(std::string name, Function function) : name{name}, function{function} {}

	virtual void visit(Visitor& visitor) override;

	std::string name;
	Function    function;
};

struct Program : Node
{
	Program(std::vector<std::unique_ptr<Node>> nodes) : nodes{std::move(nodes)} {}

	virtual void visit(Visitor& visitor) override;

	std::vector<std::unique_ptr<Node>> nodes;
};
} // namespace nodes
} // namespace ast
