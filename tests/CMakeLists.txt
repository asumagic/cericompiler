function(assembler_test name)
	add_test(
		NAME test_${name}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_test.sh
		    $<TARGET_FILE:${PROJECT_NAME}>        # Path to compiler
			${CMAKE_CURRENT_SOURCE_DIR}/${name}.p # Path to source
			${CMAKE_CURRENT_BINARY_DIR}/${name}.s # Path to output assembly
			${CMAKE_CURRENT_BINARY_DIR}/${name}   # Path to output binary
	)
endfunction()

assembler_test("test")

add_custom_target(run_unit_test ALL
	COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
	DEPENDS ${UNIT_TEST})
