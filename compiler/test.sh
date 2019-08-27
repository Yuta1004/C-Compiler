#!/bin/bash

try() {
    input="$1"
    expected="$2"

    ./yncc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo "input: $input , output: $actual"
    else
        echo "input: $input , output: $actual ... expect: $expected"
        exit 1
    fi
}

try "0;" 0
try "124;" 124
try "200;" 200
try "1+2+3+4+5+6;" 21
try "1+10+30+40-60;" 21
try "12 - 4 + 10 + 4;" 22
try "1204 - 1004;" 200
try "1 + 2 + 50 + 80 - 29 + 39;" 143
try "10*(2+3+4+5+6)/5-(3+7);" 30
try "1+2-3*4/5+(1*2*3*4);" 25
try "-3+30;" 27
try "-3*10+(5/5*100);" 70
try "-(10+20)+50;" 20
try "10+20+30 == 30+20+10;" 1
try "100/5 > 30;" 0
try "1/1+10-10 >= 0;" 1
try "20*100 < 30000;" 1
try "1 <= 0;" 0
try "a = 1; b = 2; c = 3; a = b*c/a; d = a+b+c;" 11
try "a = 10; b = 20; c = a*b; d = (c+100)/(e=10); d-e;" 20

echo "OK!"
exit 0
