#!/bin/bash

try() {
    input="$1"
    expected="$2"

    ./cc.o "$input" > tmp.s
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
try 1+2+3+4+5+6 21
try 1+10+30+40-60 21

echo "OK!"
exit 0
