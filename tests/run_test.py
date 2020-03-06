#!/usr/bin/env python3

# Usage:
# run_test.py compile_and_pray <compiler_path> <source> <asmoutput> <exeoutput>
# run_test.py compile_and_match_output <compiler_path> <source> <asmoutput> <exeoutput> <regex>
# run_test.py compile_and_match_diagnostic <compiler_path> <source> <regex>
# This should be called by a CTest within CMakeLists.txt
from subprocess import Popen, PIPE, DEVNULL
import sys
import re

action = sys.argv[1]
compiler_path = sys.argv[2]
source_path = sys.argv[3]

# TODO: dedup compile_and_pray and compile_and_match_output code

if action == "compile_and_pray":
    asm_path = sys.argv[4]
    exec_path = sys.argv[5]
    linker_path = "gcc"

    source_file = open(source_path)
    asm_file = open(asm_path, "w")

    compiler_process = Popen(
        [compiler_path],
        stdin=source_file,
        stdout=asm_file
    )

    (stdout, stderr) = compiler_process.communicate()

    if compiler_process.returncode != 0:
        sys.exit(compiler_process.returncode)

    linker_process = Popen(
        [linker_path, "-o" + exec_path, asm_path, "-no-pie"]
    )

    (stdout, stderr) = linker_process.communicate()

    if linker_process.returncode != 0:
        sys.exit(linker_process.returncode)

elif action == "compile_and_match_output":
    asm_path = sys.argv[4]
    exec_path = sys.argv[5]
    output_pattern = sys.argv[6] + '$'

    linker_path = "gcc"

    source_file = open(source_path)
    asm_file = open(asm_path, "w")

    compiler_process = Popen(
        [compiler_path],
        stdin=source_file,
        stdout=asm_file
    )

    (stdout, stderr) = compiler_process.communicate()

    if compiler_process.returncode != 0:
        sys.exit(compiler_process.returncode)

    linker_process = Popen(
        [linker_path, "-o" + exec_path, asm_path, "-no-pie"]
    )

    (stdout, stderr) = linker_process.communicate()

    if linker_process.returncode != 0:
        sys.exit(linker_process.returncode)

    output_process = Popen([exec_path], stdout=PIPE)

    (stdout, stderr) = output_process.communicate()

    if re.match(output_pattern, stdout.decode("utf-8")) is None:
        print(
            f"""Failed to match pattern "{output_pattern}". """
            f"""Program output:\n{stdout.decode("utf-8")}""",
            file=sys.stderr
        )
        sys.exit(1)

elif action == "compile_and_match_diagnostic":
    diagnostic_pattern = sys.argv[4]

    source_file = open(source_path)

    compiler_process = Popen(
        [compiler_path],
        stdin=source_file,
        stdout=DEVNULL,
        stderr=PIPE
    )

    (stdout, stderr) = compiler_process.communicate()

    if re.match(diagnostic_pattern, stderr.decode("utf-8")) is None:
        print(
            f"""Failed to match pattern "{diagnostic_pattern}". """
            f"""Compiler output:\n{stderr.decode("utf-8")}""",
            file=sys.stderr
        )
        sys.exit(1)

else:
    print(f"Invalid action {action} entered", file=sys.stderr)
    sys.exit(1)
