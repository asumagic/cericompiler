#include "expression.hpp"

#include "../visitor.hpp"

namespace ast
{
namespace nodes
{
void BinaryExpression::visit(Visitor& visitor) { visitor(*this); }
void UnaryExpression::visit(Visitor& visitor) { visitor(*this); }
void VariableExpression::visit(Visitor& visitor) { visitor(*this); }
void CallExpression::visit(Visitor& visitor) { visitor(*this); }
void TypeCastExpression::visit(Visitor& visitor) { visitor(*this); }
void StringLiteral::visit(Visitor& visitor) { visitor(*this); }
void CharacterLiteral::visit(Visitor& visitor) { visitor(*this); }
void IntegerLiteral::visit(Visitor& visitor) { visitor(*this); }
void FloatLiteral::visit(Visitor& visitor) { visitor(*this); }
} // namespace nodes
} // namespace ast
