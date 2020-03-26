#include "compiler.hpp"

#include "util/string_view.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>

int main(int argc, char** argv)
{
	// Read the source from stdin, output the assembly to stdout

	CLI::App cli{
		"cericompiler\n"
		"Example:\n"
		"\tcericompiler -i ./hello.pas -o ./hello.s\n"
		"\tas ./hello.s -o ./hello.o\n"
		"\tgcc ./hello.o -o ./hello -no-pie\n"
		"\t./hello\n"};

	std::string source_path, destination_path;
	cli.add_option("-i,--input-file", source_path, ".pas source to compile; stdin if left empty");
	cli.add_option("-s,--assembly-output", destination_path, "destination assembly file; stdout if left empty");

	// TODO: something to invoke the assembler and the linker
	// TODO: should take input files directly, not through -i

	CLI11_PARSE(cli, argc, argv);

	// possibly never used
	std::ifstream input_file;
	std::ofstream output_file;

	std::istream* input_stream  = &std::cin;
	std::ostream* output_stream = &std::cout;
	string_view   source_name   = "<stdin>";

	if (!source_path.empty())
	{
		input_stream = &input_file;
		input_file.open(source_path);
		source_name = source_path;

		if (!input_file)
		{
			fmt::print(stderr, "<cli>: could not open source file '{}' for reading\n", source_path);
			exit(1);
		}
	}

	if (!destination_path.empty())
	{
		output_stream = &output_file;
		output_file.open(destination_path);

		if (!output_file)
		{
			fmt::print(stderr, "<cli>: could not open destination file '{}' for writing\n", destination_path);
			exit(1);
		}
	}

	try
	{
		Compiler{source_name, *input_stream, *output_stream}();
	}
	catch (const std::runtime_error& e)
	{
		// Error was handled and displayed already
		exit(1);
	}
}
