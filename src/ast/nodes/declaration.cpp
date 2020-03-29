#include "declaration.hpp"

#include "../visitor.hpp"

namespace ast
{
namespace nodes
{
void VariableDeclarationBlock::visit(Visitor& visitor) { visitor(*this); }
void Program::visit(Visitor& visitor) { visitor(*this); }
void ForeignFunctionDeclaration::visit(Visitor& visitor) { visitor(*this); }
} // namespace nodes
} // namespace ast
