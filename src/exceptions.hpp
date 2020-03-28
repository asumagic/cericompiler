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
