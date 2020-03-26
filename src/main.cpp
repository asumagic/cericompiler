#include "compiler.hpp"

int main()
{
	// Read the source from stdin, output the assembly to stdout

	try
	{
		Compiler compiler;
		compiler();
	}
	catch (const std::runtime_error& e)
	{
		// Error was handled and displayed already
		exit(1);
	}
}
