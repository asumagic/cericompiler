cmake_minimum_required(VERSION 3.5)

include("3rdparty/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.251.tar.gz"
    SHA1 "5659b15dc0884d4b03dbd95710e6a1fa0fc3258d"
)

project(ceri-compiler)

hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)

if(POLICY CMP0074)
    # Fix for FLEX/CMake warning
    cmake_policy(SET CMP0074 NEW)
endif()

hunter_add_package(flex)
find_package(FLEX REQUIRED)

hunter_add_package(CLI11)
find_package(CLI11 CONFIG REQUIRED)

FLEX_TARGET(tokeniser "src/tokeniser.l" "${CMAKE_CURRENT_BINARY_DIR}/tokeniser.cpp")
add_executable(${PROJECT_NAME}
	"src/codegen/x86/codegen.cpp"
	"src/compiler.cpp"
	"src/token.cpp"
	"src/types.cpp"
	"src/main.cpp"
	"src/util/string_view.cpp"
	"src/variable.cpp"
	"tokeniser.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE "src/" ${FLEX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} fmt::fmt CLI11::CLI11)
target_compile_options(${PROJECT_NAME} PRIVATE
	"-Wall" "-Wextra"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

enable_testing()
add_subdirectory(tests)
