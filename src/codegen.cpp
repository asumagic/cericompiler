#include <iostream>

constexpr const char* main_preamble = R"(# The following lines contain the program
    .text
    .globl main
    main:
    # Save hte position of the top of the stack
    movq %rsp, %rbp

)"; // two line breaks

void emit_main_preamble()
{
    std::cout << main_preamble;
}
