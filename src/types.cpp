#include "types.hpp"

#include <array>

static constexpr std::array<string_view, 5> types{
	{"INTEGER (u64)", "DOUBLE (f64)", "BOOLEAN", "CHAR", "<arithmetic concept>"}};
static_assert(int(Type::TOTAL) == types.size(), "Please update `types` array when modifying the enum");

string_view type_name(Type type) { return types[int(type)]; }
