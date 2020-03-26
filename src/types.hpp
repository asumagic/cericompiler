#pragma once

#include "util/string_view.hpp"

#include <cstdint>
#include <limits>

enum class Type : std::int_fast32_t
{
	VOID,

	// Concrete types
	FIRST_CONCRETE,

	FIRST_ARITHMETIC = FIRST_CONCRETE,

	FIRST_INTEGRAL = FIRST_ARITHMETIC,
	UNSIGNED_INT   = FIRST_ARITHMETIC,
	LAST_INTEGRAL  = UNSIGNED_INT,

	FIRST_FLOATING,
	DOUBLE        = FIRST_FLOATING,
	LAST_FLOATING = DOUBLE,

	LAST_ARITHMETIC = LAST_FLOATING,

	BOOLEAN,

	CHAR,

	LAST_CONCRETE = CHAR,

	// Type concepts
	FIRST_CONCEPT,
	ARITHMETIC   = FIRST_CONCEPT,
	LAST_CONCEPT = ARITHMETIC,

	BUILTIN_TOTAL,

	FIRST_USER_DEFINED,
	LAST_USER_DEFINED = std::numeric_limits<std::int_fast32_t>::max(),
};

string_view type_name(Type type);
