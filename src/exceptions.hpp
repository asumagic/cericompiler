#pragma once

#include <stdexcept>

//! \brief Base class for compiler errors.
class CompilerError : public std::runtime_error
{
	public:
	using std::runtime_error::runtime_error;
};

class CompilerBug : public std::runtime_error
{
	public:
	using std::runtime_error::runtime_error;
};

class UnimplementedError : public CompilerBug
{
	public:
	using CompilerBug::CompilerBug;
};

class UnimplementedTypeSupportError : public UnimplementedError
{
	public:
	using UnimplementedError::UnimplementedError;

	UnimplementedTypeSupportError() : UnimplementedError{"Support was not implemented for this type"} {}
};
