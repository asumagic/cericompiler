# Compile and link the test ${name}.
# If the compiler or the linker returns an error code, the test fails.
function(expect_compiles name)
	add_test(
		NAME ${name}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
		    "compile_and_pray"
		    $<TARGET_FILE:${PROJECT_NAME}>        # Path to compiler
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.p # Path to source
			${CMAKE_CURRENT_BINARY_DIR}/${name}.s # Path to output assembly
			${CMAKE_CURRENT_BINARY_DIR}/${name}   # Path to output binary
	)
endfunction()

# Compile the test ${name}, but do not link it.
# The diagnostic output of the compiler must match against ${diagnostic_regex},
# otherwise the test fails.
function(expect_diagnostic name diagnostic_regex)
	add_test(
		NAME ${name}
		COMMAND
		    /usr/bin/python3
			${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
			"compile_and_match_diagnostic"
			$<TARGET_FILE:${PROJECT_NAME}>        # Path to compiler
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.p # Path to source
			${diagnostic_regex}
	)
endfunction()

# Compile and link the test ${name}.
# The output of the program must match against ${program_output_regex},
# otherwise the test fails.
function(expect_output name program_output_regex)
	add_test(
		NAME ${name}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
		    "compile_and_match_output"
			$<TARGET_FILE:${PROJECT_NAME}>        # Path to compiler
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.p # Path to source
			${CMAKE_CURRENT_BINARY_DIR}/${name}.s # Path to output assembly
			${CMAKE_CURRENT_BINARY_DIR}/${name}   # Path to output binary
			${program_output_regex}
	)
endfunction()

expect_compiles("simple-arithmetic")
expect_compiles("test-arithmetic-operators")
expect_compiles("flow-control-while")
expect_compiles("flow-control-for")
expect_compiles("multiple-typed-variables")
expect_compiles("boolean-variables")
expect_compiles("multiple-char-names")
expect_compiles("comment-test")
expect_compiles("type-char-assignment")
expect_output("big-numbers" "123\\n")
expect_diagnostic("fail-case-mismatch-binop" ".*incompatible type.*")
expect_diagnostic("fail-case-while-non-boolean" ".*incompatible type.*")
expect_diagnostic("fail-case-if-non-boolean" ".*incompatible type.*")
expect_diagnostic("fail-case-mismatch-variable" ".*incompatible type.*")
expect_diagnostic("fail-case-unknown-variable" ".*undeclared variable.*")
expect_diagnostic("fail-case-invalid-dot" ".*extraneous characters at end of file.*")
expect_diagnostic("fail-case-mismatch-boolean-in-arithmetic" ".*incompatible type.*")
expect_output("display" "0\\n")
expect_output("display-arithmetic-tests" "10\\n10")
expect_output("display-if-test" "1\\n1\\n")
expect_output("display-for-test" "1\\n2\\n3\\n4\\n5\\n")
expect_output("display-while-test" "1\\n2\\n3\\n4\\n5\\n")
expect_output("ghetto-helloworld" "Hello, world!")
expect_output("type-double-very-simple-display" "0\.0+\\n")
expect_output("type-double-assignment" "3\.1410*\\n")
expect_output("type-double-arithmetic-add" "2\.50*\\n")
expect_output("type-double-arithmetic-sub" "-0\.50*\\n")
expect_output("type-double-arithmetic-mul" "30\.00*\\n")
expect_output("type-double-arithmetic-div" "5\.00*\\n")
expect_output("type-double-arithmetic-mod" "1\.00*\\n")
expect_output("type-double-arithmetic-mixed" "-5\.00*\\n")
expect_output("type-double-comparison-megatest" "o{16}")
expect_output("type-double-convert-int" "3\\n")
expect_output("type-integer-convert-double" "123\.0+\\n")

# Force tests to occur after compilation
add_custom_target(run_unit_test ALL
	COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
	DEPENDS ${UNIT_TEST} ${PROJECT_NAME})
