#include "compiler.hpp"

#include "util/string_view.hpp"

#include <CLI/CLI.hpp>
#include <fmt/core.h>

std::string base_name(std::string path) { return path.substr(0, path.find_last_of('.')); }

struct CliFlags
{
	std::string source_path, assembly_path, program_path;
	bool        assembly_stdout, should_link = false;

	Compiler::Config config;

	void parse(CLI::App& cli, int argc, char** argv);
};

void CliFlags::parse(CLI::App& cli, int argc, char** argv)
{
	const std::map<std::string, Compiler::Target> target_map{{"x86_64-apple-darwin", Compiler::Target::APPLE_DARWIN},
															 {"x86_64-linux", Compiler::Target::LINUX}};

	// Default even if on unknown platform
	config.target = Compiler::Target::LINUX;
#ifdef __APPLE__
	config.target = Compiler::Target::APPLE_DARWIN;
#endif

	cli.add_option("input-file", source_path, ".pas source to compile; stdin if not specified");

	const auto paths_group = cli.add_option_group("output paths");

	const auto option_assembly_stdout
		= paths_group->add_flag("--assembly-stdout", assembly_stdout, "write assembly to stdout rather than a file");

	const auto option_assembly_path = paths_group->add_option(
		"-s,--assembly-output", assembly_path, "target assembly path, based on input-file if left empty");

	[[maybe_unused]] const auto option_program_path = paths_group->add_option(
		"-o,--program-output", program_path, "target program file path, a.out if left empty or unspecified");

	const auto actions_group = cli.add_option_group("actions");

	const auto option_should_link = actions_group->add_flag(
		"-l,--link", should_link, "whether an executable should be generated. enabled by --program-output");

	const auto settings_group = cli.add_option_group("compilation settings");

	[[maybe_unused]] const auto option_target
		= settings_group->add_option("--target", config.target, "target architecture and ABI")
			  ->transform(CLI::CheckedTransformer(target_map, CLI::ignore_case));

	[[maybe_unused]] const auto option_lookup_paths = settings_group->add_option(
		"-I,--include-paths",
		config.include_lookup_paths,
		"list of directories that can be used as base include directories");

	option_assembly_stdout->excludes(option_assembly_path)->excludes(option_should_link);

	cli.parse(argc, argv);

	if (!program_path.empty())
	{
		should_link = true;
	}

	if (!assembly_stdout && assembly_path.empty())
	{
		assembly_path = base_name(source_path) + ".s";
	}

	if (should_link && program_path.empty())
	{
		program_path = "a.out";
	}
}

int main(int argc, char** argv)
{
	// TODO: allow to not emit assembly
	CliFlags flags;

	{
		CLI::App cli{
			"cericompiler\n"
			"Example:\n"
			"\tcericompiler ./hello.pas -o ./hello -I /path/to/installation/std/\n"
			"\t./hello\n"};

		try
		{
			flags.parse(cli, argc, argv);
		}
		catch (const CLI::ParseError& e)
		{
			cli.exit(e);
			exit(1);
		}
	}

	{
		// possibly never used
		std::ifstream input_file;
		std::ofstream output_file;

		std::istream* input_stream  = &std::cin;
		std::ostream* output_stream = &std::cout;
		string_view   source_name   = "<stdin>";

		if (!flags.source_path.empty())
		{
			input_stream = &input_file;
			input_file.open(flags.source_path);
			source_name = flags.source_path;

			if (!input_file)
			{
				fmt::print(stderr, "<cli>: could not open source file '{}' for reading\n", flags.source_path);
				exit(1);
			}
		}

		if (!flags.assembly_path.empty())
		{
			output_stream = &output_file;
			output_file.open(flags.assembly_path);

			if (!output_file)
			{
				fmt::print(stderr, "<cli>: could not open destination file '{}' for writing\n", flags.assembly_path);
				exit(1);
			}
		}

		try
		{
			Compiler{flags.config, source_name, *input_stream, *output_stream}();
		}
		catch (const std::runtime_error& e)
		{
			// Error was handled and displayed already
			fmt::print(stderr, "<cli>: aborting due to past errors\n");
			exit(1);
		}
	}

	if (flags.should_link)
	{
		// TODO: tweakable gcc path

		const auto exit_status = std::system(
			(std::string("/usr/bin/gcc '") + flags.assembly_path + "' -o '" + flags.program_path + "' -lm").c_str());

		if (exit_status != 0)
		{
			fmt::print(stderr, "<cli>: linker unexpectedly exited with code {}\n", exit_status);
			exit(exit_status);
		}
	}
}
