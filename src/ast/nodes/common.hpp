#pragma once

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

struct Statement : Node
{};

struct Expression : Statement
{};

struct TypeName : Node
{};
} // namespace nodes
} // namespace ast
