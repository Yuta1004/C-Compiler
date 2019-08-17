#!/bin/bash

try() {
    expected="$1"
    input="$2"

    ./cc "$input" > tmp.s
    gcc -o tmp.o tmp.s
    ./tmp.o
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo "input: $input => output: $actual"
    else
        echo "input: $input => output: $actual ... expect: $expected"
        exit 1
    fi
}

try 0 0
try 124 124
try 200 200

echo "OK!"
exit 0
