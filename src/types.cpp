#include "types.hpp"

#include <array>

static constexpr std::array<const char*, 2> types{{"unsigned int", "boolean"}};

static_assert(int(Type::TOTAL) == 2);

const char* name(Type type) { return types[int(type)]; }
