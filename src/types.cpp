#include "types.hpp"

#include "util/enums.hpp"

#include <array>

static constexpr std::array<string_view, 6> types{
	{"<void>", "INTEGER (u64)", "DOUBLE (f64)", "BOOLEAN", "CHAR", "<arithmetic concept>"}};
static_assert(int(Type::BUILTIN_TOTAL) == types.size(), "Please update `types` array when modifying the enum");

string_view type_name(Type type)
{
	if (check_enum_range(type, Type::FIRST_USER_DEFINED, Type::LAST_USER_DEFINED))
	{
		// TODO: perform an actual lookup
		return "<user-defined>";
	}

	return types[int(type)];
}
