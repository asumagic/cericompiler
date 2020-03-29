#include "statement.hpp"

#include "../visitor.hpp"

namespace ast
{
namespace nodes
{
void IfStatement::visit(Visitor& visitor) { visitor(*this); }
void WhileStatement::visit(Visitor& visitor) { visitor(*this); }
void ForStatement::visit(Visitor& visitor) { visitor(*this); }
void BlockStatement::visit(Visitor& visitor) { visitor(*this); }
void DisplayStatement::visit(Visitor& visitor) { visitor(*this); }
void AssignmentStatement::visit(Visitor& visitor) { visitor(*this); }
} // namespace nodes
} // namespace ast
