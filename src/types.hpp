#pragma once

enum class Type
{
	// Concrete types
	CONCRETE_BEGIN,
	UNSIGNED_INT = CONCRETE_BEGIN,
	BOOLEAN,

	// Type concepts
	CONCEPT_BEGIN,
	ARITHMETIC = CONCEPT_BEGIN,

	TOTAL
};

const char* name(Type type);
