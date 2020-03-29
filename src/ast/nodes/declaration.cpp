#include "declaration.hpp"

#include "../visitor.hpp"

namespace ast
{
namespace nodes
{
void VariableDeclarationBlock::visit(Visitor& visitor) { visitor(*this); }
void Program::visit(Visitor& visitor) { visitor(*this); }
void ForeignFunctionDeclaration::visit(Visitor& visitor) { visitor(*this); }
void Include::visit(Visitor& visitor) { visitor(*this); }
void BuiltinType::visit(Visitor& visitor) { visitor(*this); }
void UserType::visit(Visitor& visitor) { visitor(*this); }

} // namespace nodes
} // namespace ast
