function(expect_compiles name)
	add_test(
		NAME ${name}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
		    "compile_and_pray"
		    $<TARGET_FILE:${PROJECT_NAME}>        # Path to compiler
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.p # Path to source
			${CMAKE_CURRENT_BINARY_DIR}/${name}.s # Path to output assembly
			${CMAKE_CURRENT_BINARY_DIR}/${name}   # Path to output binary
			${action}
	)
endfunction()

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

expect_compiles("simple-arithmetic")
expect_compiles("test-arithmetic-operators")
expect_compiles("flow-control-while")
expect_compiles("flow-control-for")
expect_diagnostic("fail-case-mismatch-binop" ".*incompatible type.*")
expect_diagnostic("fail-case-while-non-boolean" ".*incompatible type.*")
expect_diagnostic("fail-case-if-non-boolean" ".*incompatible type.*")
expect_diagnostic("fail-case-mismatch-variable" ".*incompatible type.*")
expect_diagnostic("fail-case-unknown-variable" ".*variable '.*' not found.*")
expect_diagnostic("fail-case-invalid-dot" ".*extraneous characters at end of file.*")
expect_diagnostic("fail-case-mismatch-boolean-in-arithmetic" ".*incompatible type.*")

# Force tests to occur after compilation
add_custom_target(run_unit_test ALL
	COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
	DEPENDS ${UNIT_TEST} ${PROJECT_NAME})
