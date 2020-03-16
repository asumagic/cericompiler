#include "types.hpp"

#include <array>

static constexpr std::array<string_view, 4> types{{"unsigned int", "boolean", "char", "Arithmetic concept"}};
static_assert(int(Type::TOTAL) == types.size(), "Please update `types` array when modifying the enum");

string_view type_name(Type type) { return types[int(type)]; }
