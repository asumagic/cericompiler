# CERIcompiler

A simple compiler.
From: Pascal-like imperative LL(k) langage
To: x86-64 AT&T syntax assembly.

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
