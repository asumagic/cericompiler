#pragma once

#include <memory>

namespace ast
{
class Visitor;

namespace nodes
{
struct Node
{
	virtual ~Node() = default;

	virtual void visit(Visitor& visitor) = 0;
};

struct TypeName : Node
{};

struct Statement : Node
{};

struct Expression : Statement
{
	std::unique_ptr<TypeName> baked_type;
};
} // namespace nodes
} // namespace ast
