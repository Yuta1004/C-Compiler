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
try "a = 1; b = 2; c = 3; a = b*c/a; d = a+b+c; return d;" 11
try "a = 10; b = 20; c = a*b; d = (c+100)/(e=10); return d-e;" 20
try "one = 1; two = 2; sum = one + two; return sum;" 3
try "a_result = 1204; b_result = 1004; ret_value = a_result - b_result; return ret_value;" 200
try "a = 10; b = 20; tmp = a; a = b; b = tmp; return a;" 20
try "a = 10; b = 20; tmp = a; a = b; b = tmp; return b;" 10
try "seven = 7; eleven = 11; seven_eleven = seven + eleven; return seven_eleven;" 18
try "return a=b=10;" 10
try "a = 10; b = 30; if(a == 10) return b;" 30
try "a = 10; b = 10; if(a == b) return 20; else return 100;" 20
try "a = 10; b = 10; if(a != b) return 20; else return 100;" 100
try "result = 0; if (1) if (1) if (0) result = result + 10; else if(1) result = result + 20; else result = result + 30; else result = result + 40; return result;" 20
try "num = 0; while(num < 10) num = num + 1; return num;" 10

echo "OK!"
exit 0
