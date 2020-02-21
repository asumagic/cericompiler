#!/bin/sh

# usage: run_test.sh /path/to/compiler source.p source.s target_executable

$1 <$2 >$3 &&\
gcc $3 -o $4 -no-pie
