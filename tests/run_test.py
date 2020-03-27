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

linker_flags = ["-no-pie", "-lm"]

# TODO: dedup compile_and_pray and compile_and_match_output code
# TODO: check and print error codes

import os
common_compiler_flags = [
    # TODO: retrieve include path in an actually sane way
    "-I" + os.path.dirname(source_path) + "/../std/"
]

if action == "compile_and_pray":
    asm_path = sys.argv[4]
    exec_path = sys.argv[5]

    compiler_process = Popen([
        compiler_path,
        source_path,
        "--assembly-output", asm_path,
        "--program-output", exec_path,
        *common_compiler_flags
    ])

    (stdout, stderr) = compiler_process.communicate()

    if compiler_process.returncode != 0:
        sys.exit(compiler_process.returncode)

elif action == "compile_and_match_output":
    asm_path = sys.argv[4]
    exec_path = sys.argv[5]
    output_pattern = sys.argv[6] + '$'

    compiler_process = Popen([
        compiler_path,
        source_path,
        "--assembly-output", asm_path,
        "--program-output", exec_path,
        *common_compiler_flags
    ])

    (stdout, stderr) = compiler_process.communicate()

    if compiler_process.returncode != 0:
        sys.exit(compiler_process.returncode)

    output_process = Popen([exec_path], stdout=PIPE)

    (stdout, stderr) = output_process.communicate()

    if re.match(output_pattern, stdout.decode("utf-8")) is None:
        print(
            "Failed to match pattern \"{}\". ".format(output_pattern) +
            "Program output:\n{}".format(stdout.decode("utf-8")),
            file=sys.stderr
        )
        sys.exit(1)

elif action == "compile_and_match_diagnostic":
    diagnostic_pattern = sys.argv[4]

    compiler_process = Popen(
        [
            compiler_path,
            source_path,
            "--assembly-stdout", # TODO: option to discard assembly
            *common_compiler_flags
        ],
        stdout=DEVNULL,
        stderr=PIPE
    )

    (stdout, stderr) = compiler_process.communicate()

    if re.match(diagnostic_pattern, stderr.decode("utf-8")) is None:
        print(
            "Failed to match pattern \"{}\". ".format(diagnostic_pattern) +
            "Compiler output:\n{}".format(stderr.decode("utf-8")),
            file=sys.stderr
        )
        sys.exit(1)

else:
    print("Invalid action {} entered".format(action), file=sys.stderr)
    sys.exit(1)
