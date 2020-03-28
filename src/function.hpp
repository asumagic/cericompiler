#pragma once

#include "types.hpp"

#include <vector>
#include <string>

struct FunctionParameter
{
	Type        type;
	std::string name = "";
};

struct Function
{
	bool                           variadic = false;
	Type                           return_type;
	std::vector<FunctionParameter> parameters;
	bool                           foreign = false;
};
