# CERIcompiler

A simple compiler.
From: Pascal-like imperative LL(k) langage
To: 64 bit 80x86 assembly langage (AT&T)

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

## Usage

`ceri-compiler` reads the source from `stdin` and writes at&t x86-64 assembly to `stdout`.

Building should run tests, some of which dump the assembly files in the `tests/` subdirectory *within your build directory*.
