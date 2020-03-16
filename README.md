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

## Usage

`ceri-compiler` reads the source from `stdin` and writes at&t x86-64 assembly to `stdout`.

Building should run tests, some of which dump the assembly files in the `tests/` subdirectory *within your build directory*.

The generated assembly requires to be linked against the C standard library.
Note that the generated assembly uses the SystemV ABI (which Windows does not use).

## Implementation status

Types:
- [x] Typed variables
- [x] `INTEGER` type
- [ ] `CHAR` type
- [ ] `DOUBLE` type
- [x] `BOOLEAN` type
- [x] Explicit type conversions
    - [x] Integral <=> Integral (e.g. no-op or `CHAR` <=> `INTEGER`)
    - [ ] Integral <=> Floating-point

Operators:
- [x] Arithmetic operators: `+`, `-`, `*`, `/`, `%`
    - [x] Integral
    - [ ] Floating-point
- [x] Logical operators: `||`, `&&`
- [ ] Boolean negation operator: `!`
- [x] Comparison operators: `<`, `<=`, `==`, `>`, `>=`, `!=` (or `<>`)
    - [x] Integral
    - [ ] Floating-point

Statements:
- [x] `IF` statement
- [x] `FOR` statement
    - [x] `TO` support
    - [ ] `DOWNTO` support
- [x] `WHILE` statement
- [ ] `CASE` statement
- [x] `DISPLAY` debug statement

Procedures:
- [ ] User procedure support
    - [ ] Declaration support
    - [ ] Calling support
    - [ ] Local variables
    - [ ] Parameter support
    - [ ] Return value support
- [ ] C Foreign function interface

## Language grammar

```sf
Letter                    := "a"|...|"z"

Number                    := Digit{Digit}
Digit                     := "0"|...|"9"

Factor                    := Number | Letter | "(" Expression ")"| "!" Factor | TypeCast
TypeCast                  := Type "(" Expression ")"

Term                      := Factor {MultiplicativeOperator Factor}
MultiplicativeOperator    := "*" | "/" | "%" | "&&"

SimpleExpression          := Term {AdditiveOperator Term}
AdditiveOperator          := "+" | "-" | "||"

DeclarationPart           := "VAR" VarDeclaration {";" VarDeclaration} "."
VarDeclaration            := Ident {"," Ident} ":" Type

Type                      := "INTEGER" | "BOOLEAN"

Expression                := SimpleExpression [RelationalOperator SimpleExpression]
RelationalOperator        := "==" | "!=" | "<" | ">" | "<=" | ">="

AssignementStatement      := Identifier ":=" Expression

IfStatement               := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
WhileStatement            := "WHILE" Expression DO Statement
ForStatement              := "FOR" AssignementStatement "TO" Expression "DO" Statement
BlockStatement            := "BEGIN" Statement { ";" Statement } "END"
DisplayStatement          := "DISPLAY" Expression

Statement                 := AssignementStatement
StatementPart             := Statement {";" Statement} "."
Program                   := [DeclarationPart] StatementPart
```
