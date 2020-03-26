# CERIcompiler

A simple compiler for a Pascal-like language.

The supported backends currently are:
- x86-64 assembly (AT&T).

## Requirements

- CMake 3.5 or later
- A C++11 compatible compiler
- (optional) Python 3 for running tests

## Building

```sh
mkdir build
cd build
cmake ..
make -j
```

Dependencies should be fetched automatically by the Hunter package manager.
Note that, as a result, building this project will create a `.hunter` directory in your home directory, which can then be removed safely.

### Dependencies

The following dependencies are in use but do not require manual installation:
- [Hunter](https://github.com/cpp-pm/hunter) (C++ package manager)
- [CLI11](https://github.com/CLIUtils/CLI11) (commandline parsing)
- [FLEX](https://github.com/westes/flex) (scanner generator)
- [\{fmt\}](https://github.com/fmtlib/fmt) (formatting library)

## Usage

`ceri-compiler` reads the source from `stdin` and writes at&t x86-64 assembly to `stdout`.

Building should run tests, some of which dump the assembly files in the `tests/` subdirectory *within your build directory*.

The generated assembly requires to be linked against the C standard library.
Note that the generated assembly uses the SystemV ABI (which Windows does not use).

## Implementation status

Types:
- [x] Typed variables
- [x] `INTEGER` type
    - [x] Integer literals
- [x] `CHAR` type
    - [x] Character literals
- [x] `DOUBLE` type
    - [x] Floating-point literals
- [x] `BOOLEAN` type
- [x] Explicit type conversions
    - [x] Integral <=> Integral (e.g. no-op or `CHAR` <=> `INTEGER`)
    - [x] Integral <=> Floating-point
- [ ] User-defined types
    - [x] `TYPE alias = aliased` syntax
    - [ ] Records
- [x] Pointer types
    - [x] Pointer to user types (e.g. pointer to pointer)
- [ ] Dynamic allocation
- [ ] Arrays

Operators:
- [x] Arithmetic operators: `+`, `-`, `*`, `/`, `%`
    - [x] Integral
    - [x] Floating-point
- [x] Logical operators: `||`, `&&`
- [x] Boolean negation operator: `!`
- [x] Comparison operators: `<`, `<=`, `==`, `>`, `>=`, `!=` (or `<>`)
    - [x] Integral
    - [x] Floating-point

Statements:
- [x] `IF` statement
- [x] `FOR` statement
    - [x] `TO` support
    - [ ] `DOWNTO` support
- [x] `WHILE` statement
- [ ] `CASE` statement
- [x] `DISPLAY` debug statement

Functions:
- [ ] Function support
    - [x] C foreign function interface
        - [x] Declaration support
        - [x] Calling support
        - [x] Parameter support
        - [x] Return value support
    - [ ] User-defined functions
        - [ ] Declaration support
        - [ ] Calling support
        - [ ] Local variables
        - [ ] Parameter support
        - [ ] Return value support
        - [ ] Generic procedures

Misc:
- [x] Include file support
- [ ] C99 standard library bindings (when appropriate): Typedefs, constants and functions under `stdc/`
    - [x] `math.h` (partial)
    - [ ] [Others](https://en.cppreference.com/w/c/header)

## Language grammar

This is provided for demonstration purposes, and includes some approximations (e.g. `Symbol` is not strictly defined and
`Number` is underspecified).

```sf
Identifier                 := Letter{Letter|Digit|"_"}
Letter                     := "a"|...|"z"

Number                     := Digit{Digit}
Digit                      := "0"|...|"9"

StringLiteral              := "\"" {Symbol} "\""
CharacterLiteral           := "'" Symbol "'"
IntegerLiteral             := Number
FloatLiteral               := Number "." Number
Literal                    := CharacterLiteral | IntegerLiteral | FloatLiteral

TypeCast                   := "CONVERT" Expression "TO" Type

Dereferencable             := Literal
                            | "@" Identifier
                            | Identifier
                            | "(" Expression ")"
                            | "!" Factor
                            | TypeCast
                            | FunctionCall

Factor                     := Dereferencable { "^" }

FunctionCall               := Identifier "(" ParamList ")"
ParamList                  := [ Expression {"," Expression} ]

Term                       := Dereferencable {MultiplicativeOperator Dereferencable}
MultiplicativeOperator     := "*" | "/" | "%" | "&&"

SimpleExpression           := Term {AdditiveOperator Term}
AdditiveOperator           := "+" | "-" | "||"

VarDeclarationBlock        := "VAR" VarDeclaration {VarDeclaration}
VarDeclaration             := Identifier {"," Identifier} ":" Type ";"

TypeDeclaration            := "TYPE" Identifier "=" Type ";"

ForeignFunctionDeclaration := "FFI" Identifier "(" TypeList ")" : TypeOrVoid
TypeList                   := [ Type {"," Type} ]

Include                    := "INCLUDE" StringLiteral ";"

Declaration                := VarDeclarationBlock
                            | TypeDeclaration
                            | ForeignFunctionDeclaration
                            | Include

Type                       := "INTEGER" | "CHAR" | "BOOLEAN" | "DOUBLE" | Identifier | PointerType
PointerType                := "^" Type
TypeOrVoid                 := Type | "VOID"

Expression                 := SimpleExpression [RelationalOperator SimpleExpression]
RelationalOperator         := "==" | "!=" | "<>" | "<" | ">" | "<=" | ">="

AssignementStatement       := Identifier { "^" } ":=" Expression
IfStatement                := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
WhileStatement             := "WHILE" Expression DO Statement
ForStatement               := "FOR" AssignementStatement "TO" Expression "DO" Statement
BlockStatement             := "BEGIN" [ Statement { ";" Statement } [";"] ] "END"
DisplayStatement           := "DISPLAY" Expression

Statement                  := FunctionCall
                            | AssignementStatement
                            | IfStatement
                            | WhileStatement
                            | ForStatement
                            | BlockStatement
                            | DisplayStatement
                            | TypeDefinition

MainBlockStatement         := BlockStatement

Program                    := {Declaration} MainBlockStatement "."
```
