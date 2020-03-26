#include "variable.hpp"

#include <fmt/core.h>

std::string Variable::mangled_name() const { return fmt::format("_ceri_var_{}", name); }
