#pragma once

#include <stdexcept>

//! \brief Class to propagate a compiler error.
//!
//! \details
//!		These are thrown only to indicate a compiler error was issued, i.e. error() was called.
//!		These are meant to be handled when parsing to provide additional context.
//!		A generic CompilerError handler must not call error(), this is not its purpose.
class CompilerError : public std::runtime_error
{
	public:
	using std::runtime_error::runtime_error;
};

// TODO: this isn't great and only necessary because CodeGen cannot call Compiler's bug(), but it should be able to.
//! \brief Class to propagate a compiler bug to the Compiler exception handler.
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
