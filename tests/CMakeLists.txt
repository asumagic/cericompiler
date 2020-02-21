function(assembler_test path)
	add_test(
		NAME test_${path}
		COMMAND run_test.sh ${path}
	)
endfunction()

assembler_test("tests/test.p")

add_custom_target(run_unit_test ALL
	COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
	DEPENDS ${UNIT_TEST})
