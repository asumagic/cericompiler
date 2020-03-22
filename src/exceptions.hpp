#pragma once

#include <stdexcept>

//! \brief Base class for compiler errors.
class CompilerError : public std::runtime_error
{
	public:
	using std::runtime_error::runtime_error;
};

class UnimplementedError : public CompilerError
{
	public:
	using CompilerError::CompilerError;
};

class UnimplementedTypeSupportError : public UnimplementedError
{
	public:
	using UnimplementedError::UnimplementedError;

	UnimplementedTypeSupportError() : UnimplementedError{"Support was not implemented for this type"} {}
};
