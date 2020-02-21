#pragma once

enum class Type
{
	UNSIGNED_INT,
	BOOLEAN,

	TOTAL
};

const char* name(Type type);
