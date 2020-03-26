#pragma once

#include "types.hpp"

#include <string>

struct VariableType
{
	// This is a struct on its own in case we add metadata or anything of the sort
	Type type;
};

struct Variable
{
	std::string  name;
	VariableType type;

	std::string mangled_name() const;
};
