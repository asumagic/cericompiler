cmake_minimum_required(VERSION 3.5)

project(ceri-compiler)

find_package(FLEX)
FLEX_TARGET(tokeniser "src/tokeniser.l" "${CMAKE_CURRENT_BINARY_DIR}/tokeniser.cpp")

add_executable(${PROJECT_NAME}
	"src/codegen.cpp"
	"src/compiler.cpp"
	"src/types.cpp"
	"src/main.cpp"
	"tokeniser.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE "src/")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

enable_testing()
add_subdirectory(tests)
