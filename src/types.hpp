#pragma once

#include "util/string_view.hpp"

enum class Type
{
	// Concrete types
	FIRST_CONCRETE,

	FIRST_ARITHMETIC = FIRST_CONCRETE,
	UNSIGNED_INT     = FIRST_ARITHMETIC,
	LAST_ARITHMETIC  = UNSIGNED_INT,

	BOOLEAN,

	LAST_CONCRETE = BOOLEAN,

	// Type concepts
	FIRST_CONCEPT,
	ARITHMETIC   = FIRST_CONCEPT,
	LAST_CONCEPT = ARITHMETIC,

	TOTAL
};

string_view type_name(Type type);
