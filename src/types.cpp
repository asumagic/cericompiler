#include "types.hpp"

#include <array>

static constexpr std::array<const char*, 3> types{{"unsigned int", "boolean", "Arithmetic concept"}};
static_assert(int(Type::TOTAL) == types.size(), "Please update `types` array when modifying the enum");

const char* type_name(Type type) { return types[int(type)]; }
