#!/bin/bash

try() {
    input="$1"
    expected="$2"

    ./yncc "$input" > tmp.s
    gcc -c -o test_func.o test_func.c
    gcc -O0 -g -o tmp tmp.s test_func.o
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo -e "\e[32mSuccess\e[m $input"
    else
        echo -e "\e[1;31mFail    input: $input => output: $actual ... expect: $expected\e[m"
    fi
}

# exprssion
echo -e "\e[1m\nExprssion\e[m"
try "main(){ return 0; }" 0
try "main(){ return 124; }" 124
try "main(){ return 200; }" 200
try "main(){ return 1+2+3+4+5+6; }" 21
try "main(){ return 1+10+30+40-60; }" 21
try "main(){ return 12 - 4 + 10 + 4; }" 22
try "main(){ return 1204 - 1004; }" 200
try "main(){ return 1 + 2 + 50 + 80 - 29 + 39; }" 143
try "main(){ return 10*(2+3+4+5+6)/5-(3+7); }" 30
try "main(){ return 1+2-3*4/5+(1*2*3*4); }" 25
try "main(){ return -3+30; }" 27
try "main(){ return -3*10+(5/5*100); }" 70
try "main(){ return -(10+20)+50; }" 20
try "main(){ return 10+20+30 == 30+20+10; }" 1
try "main(){ return 100/5 > 30; }" 0
try "main(){ return 1/1+10-10 >= 0; }" 1
try "main(){ return 20*100 < 30000; }" 1
try "main(){ return 1 <= 0; }" 0
try "main(){ return 10 % 3; }" 1

# variable
echo -e "\e[1m\nVariable\e[m"
try "main(){ int a; int b; int c; int d; a = 1; b = 2; c = 3; a = b*c/a; d = a+b+c; return d; }" 11
try "main(){ int a; int b; int c; int d; int e; a = 10; b = 20; c = a*b; d = (c+100)/(e=10); return d-e; }" 20
try "main(){ int one; int two; int sum; one = 1; two = 2; sum = one + two; return sum; }" 3
try "main(){ int a_result; int b_result; int ret_value; a_result = 1204; b_result = 1004; ret_value = a_result - b_result; return ret_value; }" 200
try "main(){ int a; int b; int tmp; a = 10; b = 20; tmp = a; a = b; b = tmp; return a; }" 20
try "main(){ int a; int b; int tmp; a = 10; b = 20; tmp = a; a = b; b = tmp; return b; }" 10
try "main(){ int seven; int eleven; int seven_eleven; seven = 7; eleven = 11; seven_eleven = seven + eleven; return seven_eleven; }" 18
try "main(){ int a; int b; return a=b=10; }" 10

# if
echo -e "\e[1m\nif\e[m"
try "main(){ int a; int b; a = 10; b = 30; if(a == 10) return b; }" 30
try "main(){ int a; int b; a = 10; b = 10; if(a == b) return 20; else return 100; }" 20
try "main(){ int a; int b; a = 10; b = 10; if(a != b) return 20; else return 100; }" 100
try "main(){ int result; result = 0; if (1) if (1) if (0) result = result + 10; else if(1) result = result + 20; else result = result + 30; else result = result + 40; return result; }" 20

# while
echo -e "\e[1m\nwhile\e[m"
try "main(){ int num; num = 0; while(num < 10) num = num + 1; return num; }" 10

# for
echo -e "\e[1m\nfor\e[m"
try "main(){ int num; int i; num = 0; for(i = 0; i < 200; i = i + 1) num = num + 1; return num; }" 200
try "main(){ int num; num = 0; for(; num < 100; ) num = num + 1; return num; }" 100
try "main(){ int a; a = 0; for(;;) if(a == 10) return a; else a = a + 1; return a; }" 10
try "main(){ int a; a = 0; for(;; a = a + 1) if(a == 10) return a; }" 10
try "main(){ int sum; int n; sum = 0; for(n = 1; n <= 10; n = n + 1) sum = sum + n; return sum; }" 55
try "main(){ int sum; int n; sum = 0; for(n = 0; n <= 20; n = n + 2) sum = sum + n; return sum; }" 110
try "main(){ int sum; int n; sum = 0; for(n = 1; n <= 20; n = n + 1) if(n % 2 == 0) sum = sum + n; return sum; }" 110

# block
echo -e "\e[1m\nblock\e[m"
try "main(){ int cnt; cnt = 0; for(; cnt < 10;){ cnt = cnt + 1; } return cnt; }" 10
try "main(){ int a; int b; int cnt; int tmp; a = 1; b = 1; for(cnt = 0; cnt < 10; cnt = cnt + 1){ tmp = a; a = b; b = tmp + b; } return b; }" 144
try "main(){ int result; result = 0; if(1){ if(1){ if(1){ result = result + 1; result = result + 2;} result = result + 3;} result = result + 4;} return result; }" 10

# call function
echo -e "\e[1m\ncall function\e[m"
try "main(){ return funcA(); }" 0
try "main(){ int a; int b; a = funcA() + 10; b = 20; return a+b; }" 30
try "main(){ funcB(9); }" 0
try "main(){ funcC(1, 2, 3, 4); }" 0
try "main(){ funcC(1, 0, 0, 2); }" 0
try "main(){ funcD(1, 2, 3, 4, 5, 6); }" 0
try "main(){ funcD(10, 20, 30, 40, 50, 60); }" 0
try "main(){ funcE(1, 3, 5, 7); }" 0
try "main(){ return funcF(8, 5, 9, 4, 7, 1); }" 34
try "main(){ int sum; sum = funcF(1, 2, 3, 4, 5, 6); print(1, sum); return 0; }" 0
try "main(){ int sum; int num; sum = 0; for(num = 1; num <= 10; num = num + 1){ sum = sum + num; print(1, sum); } return 0; }" 0

# define function
echo -e "\e[1m\ndefine function\e[m"
try "main(){ return sum(1, 2); } sum(a, b){ return a+b; }" 3
try "main(){ int result; result = sum(10, 20); return result - 15; }  sum(a, b){ return a+b; }" 15
try "main(){ int a; a = 10; int b; int result; b = 20; result = sum(a, b); return result - 15; }  sum(a, b){ return a+b; }" 15
try "main(){ int a; a = 10; int b; int result; b = 20; result = sum(a, b); return result; } sum(a, b){ return a+b; }" 30
try "main(){ int ans; int idx; int ans; ans = 0; for(idx = 1; idx <= 10; idx = idx + 1){ ans = sum(ans, idx); } return ans; } sum(a, b){ return a+b; }" 55
try "main(){ return mul(sum(1, 2), sum(3, 4)); } sum(a, b){ return a+b; } mul(a, b){ return a*b; }" 21
try "main(){ return sum(sum(sum(1, 2), sum(3, 4)), sum(sum(5, 6), sum(7, 8))); } sum(a, b){ return a+b; }" 36
try "main(){ int num; for(num = 1; num <= 10; num = num + 1){ print(2, num, fib(num)); }} fib(depth){ if(depth <= 2){ return 1; } else { return fib(depth-1) + fib(depth-2); }}" 0

# pointer
echo -e "\e[1m\npointer\e[m"
try "main(){ int a; int ap; int apv; a = 4; ap = &a; apv = *ap; print(2, ap, apv); return 0; }" 0
try "main(){ int x; int y; int z; int zc; x = 3; y = 5; z = &y + 8; zc = *z; print(1, zc); return zc; }" 3
try "main(){ int c; int y; int tmp; c = 1204; y = 1004; tmp = *(&y + 8); print(1, tmp); return 0;} " 0

echo ""
exit 0
