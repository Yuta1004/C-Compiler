#!/bin/bash

# Setup
gcc -c -o test_func.o test_func.c

# Try Function
try() {
    input="$1"
    expected="$2"

    echo "$input" > program.c
    ./yncc program.c > tmp.s
    gcc -no-pie -O0 -g -o tmp tmp.s test_func.o
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ]; then
        echo -e "\e[1;32mSuccess\e[m $input => $actual"
    else
        echo -e "\e[1;31mFail    input: $input => output: $actual ... expect: $expected\e[m"
    fi
}

# Test
## exprssion
echo -e "\e[1m\nexprssion\e[m"
try "int main(){ return 0; }" 0
try "int main(){ return 124; }" 124
try "int main(){ return 200; }" 200
try "int main(){ return 1+2+3+4+5+6; }" 21
try "int main(){ return 1+10+30+40-60; }" 21
try "int main(){ return 12 - 4 + 10 + 4; }" 22
try "int main(){ return 1204 - 1004; }" 200
try "int main(){ return 1 + 2 + 50 + 80 - 29 + 39; }" 143
try "int main(){ return 10*(2+3+4+5+6)/5-(3+7); }" 30
try "int main(){ return 1+2-3*4/5+(1*2*3*4); }" 25
try "int main(){ return -3+30; }" 27
try "int main(){ return -3*10+(5/5*100); }" 70
try "int main(){ return -(10+20)+50; }" 20
try "int main(){ return 10+20+30 == 30+20+10; }" 1
try "int main(){ return 100/5 > 30; }" 0
try "int main(){ return 1/1+10-10 >= 0; }" 1
try "int main(){ return 20*100 < 30000; }" 1
try "int main(){ return 1 <= 0; }" 0
try "int main(){ return 10 % 3; }" 1
try "int main(){ return 100 / 100; }" 1

## variable
echo -e "\e[1m\nvariable\e[m"
try "int main(){ int a; int b; int c; int d; a = 1; b = 2; c = 3; a = b*c/a; d = a+b+c; return d; }" 11
try "int main(){ int a; int b; int c; int d; int e; a = 10; b = 20; c = a*b; d = (c+100)/(e=10); return d-e; }" 20
try "int main(){ int one; int two; int sum; one = 1; two = 2; sum = one + two; return sum; }" 3
try "int main(){ int a_result; int b_result; int ret_value; a_result = 1204; b_result = 1004; ret_value = a_result - b_result; return ret_value; }" 200
try "int main(){ int a; int b; int tmp; a = 10; b = 20; tmp = a; a = b; b = tmp; return a; }" 20
try "int main(){ int a; int b; int tmp; a = 10; b = 20; tmp = a; a = b; b = tmp; return b; }" 10
try "int main(){ int seven; int eleven; int seven_eleven; seven = 7; eleven = 11; seven_eleven = seven + eleven; return seven_eleven; }" 18
try "int main(){ int a; int b; return a=b=10; }" 10

## if
echo -e "\e[1m\nif\e[m"
try "int main(){ int a; int b; a = 10; b = 30; if(a == 10) return b; }" 30
try "int main(){ int a; int b; a = 10; b = 10; if(a == b) return 20; else return 100; }" 20
try "int main(){ int a; int b; a = 10; b = 10; if(a != b) return 20; else return 100; }" 100
try "int main(){ int result; result = 0; if (1) if (1) if (0) result = result + 10; else if(1) result = result + 20; else result = result + 30; else result = result + 40; return result; }" 20

## while
echo -e "\e[1m\nwhile\e[m"
try "int main(){ int num; num = 0; while(num < 10) num = num + 1; return num; }" 10

## for
echo -e "\e[1m\nfor\e[m"
try "int main(){ int num; int i; num = 0; for(i = 0; i < 200; i = i + 1) num = num + 1; return num; }" 200
try "int main(){ int num; num = 0; for(; num < 100; ) num = num + 1; return num; }" 100
try "int main(){ int a; a = 0; for(;;) if(a == 10) return a; else a = a + 1; return a; }" 10
try "int main(){ int a; a = 0; for(;; a = a + 1) if(a == 10) return a; }" 10
try "int main(){ int sum; int n; sum = 0; for(n = 1; n <= 10; n = n + 1) sum = sum + n; return sum; }" 55
try "int main(){ int sum; int n; sum = 0; for(n = 0; n <= 20; n = n + 2) sum = sum + n; return sum; }" 110
try "int main(){ int sum; int n; sum = 0; for(n = 1; n <= 20; n = n + 1) if(n % 2 == 0) sum = sum + n; return sum; }" 110

## block
echo -e "\e[1m\nblock\e[m"
try "int main(){ int cnt; cnt = 0; for(; cnt < 10;){ cnt = cnt + 1; } return cnt; }" 10
try "int main(){ int a; int b; int cnt; int tmp; a = 1; b = 1; for(cnt = 0; cnt < 10; cnt = cnt + 1){ tmp = a; a = b; b = tmp + b; } return b; }" 144
try "int main(){ int result; result = 0; if(1){ if(1){ if(1){ result = result + 1; result = result + 2;} result = result + 3;} result = result + 4;} return result; }" 10

## call function
echo -e "\e[1m\ncall function\e[m"
try "int main(){ return funcA(); }" 0
try "int main(){ int a; int b; a = funcA() + 10; b = 20; return a+b; }" 30
try "int main(){ funcB(9); }" 0
try "int main(){ funcC(1, 2, 3, 4); }" 0
try "int main(){ funcC(1, 0, 0, 2); }" 0
try "int main(){ funcD(1, 2, 3, 4, 5, 6); }" 0
try "int main(){ funcD(10, 20, 30, 40, 50, 60); }" 0
try "int main(){ funcE(1, 3, 5, 7); }" 0
try "int main(){ return funcF(8, 5, 9, 4, 7, 1); }" 34
try "int main(){ int sum; sum = funcF(1, 2, 3, 4, 5, 6); print(1, sum); return 0; }" 0
try "int main(){ int sum; int num; sum = 0; for(num = 1; num <= 10; num = num + 1){ sum = sum + num; print(1, sum); } return 0; }" 0

## define function
echo -e "\e[1m\ndefine function\e[m"
try "int main(){ return sum(1, 2); } int sum(int a, int b){ return a+b; }" 3
try "int main(){ int result; result = sum(10, 20); return result - 15; }  int sum(int a, int b){ return a+b; }" 15
try "int main(){ int a; a = 10; int b; int result; b = 20; result = sum(a, b); return result - 15; }  int sum(int a, int b){ return a+b; }" 15
try "int main(){ int a; a = 10; int b; int result; b = 20; result = sum(a, b); return result; } int sum(int a, int b){ return a+b; }" 30
try "int main(){ int ans; int idx; int ans; ans = 0; for(idx = 1; idx <= 10; idx = idx + 1){ ans = sum(ans, idx); } return ans; } int sum(int a, int b){ return a+b; }" 55
try "int main(){ return mul(sum(1, 2), sum(3, 4)); } int sum(int a, int b){ return a+b; } int mul(int a, int b){ return a*b; }" 21
try "int main(){ return sum(sum(sum(1, 2), sum(3, 4)), sum(sum(5, 6), sum(7, 8))); } int sum(int a, int b){ return a+b; }" 36
try "int main(){ int num; for(num = 1; num <= 10; num = num + 1){ print(2, num, fib(num)); }} int fib(int depth){ if(depth <= 2){ return 1; } else { return fib(depth-1) + fib(depth-2); }}" 0

## pointer
echo -e "\e[1m\npointer\e[m"
try "int main(){ int a; int *ap; int apv; a = 4; ap = &a; apv = *ap; print(2, ap, apv); return 0; }" 0
try "int main(){ int x; int y; int *z; int zc; x = 3; y = 5; z = &y + 1; zc = *z; print(1, zc); return zc; }" 3
try "int main(){ int a; int *p; a = 10; p = &a; return *p; }" 10
try "int main(){ int x; int *y; y = &x; *y = 3; return x; }" 3
try "int main(){ int x; int *y; int **z; x = 100; y = &x; z = &y; **z = 10; return x; }" 10
try "int main(){ int *p; mem_alloc(&p, 4, 1, 2, 3, 4); int *q; q = p + 2; return *q; }" 3
try "int main(){ int *p; mem_alloc(&p, 4, 1, 2, 4, 8); int *q; q = p + 3; return *q; }" 8
try "int main(){ int *p; mem_alloc(&p, 4, 1, 3, 5, 7); int *q; int idx; for(idx = 0; idx < 4; idx = idx + 1){ q = p + idx; print(1, *q); } return 0; }" 0

## sizeof
echo -e "\e[1m\nsizeof\e[m"
try "int main(){ int x; return sizeof(x); }" 4
try "int main(){ int *y; return sizeof(y); }" 8
try "int main(){ int x; return sizeof(x + 3); }" 4
try "int main(){ int *y; return sizeof(y + 1); }" 8
try "int main(){ return sizeof(100); }" 4
try "int main(){ return sizeof(sizeof(100)); }" 4
try "int main(){ return sizeof sizeof sizeof sizeof sizeof sizeof 10; }" 4
try "int main(){ int x; print(1, sizeof(x)); return 0; }" 0
try "int main(){ int nums[12]; return sizeof(nums)/sizeof(nums[0]); }" 12
try "int nums[6]; int main(){ return sizeof(nums)/sizeof(nums[0]); }" 6
try "int main(){ int a[10]; int *b = a; return sizeof(a+2) + sizeof(b); }" 16
try "int main(){ int a[10]; int *b = a; return sizeof(a) + sizeof(b); }" 48

## array
echo -e "\e[1m\narray\e[m"
try "int main(){ int a; int b; int array[10]; }" 0
try "int main(){ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; int num; num = *(p + 1); print(1, num); return *p + *(p + 1); }" 3
try "int main(){ int array[10]; int idx; int num; for(idx = 0; idx < 10; idx = idx + 1){ *(array + idx) = idx * 10; } for(idx = 0; idx < 10; idx = idx + 1){ num = *(array + idx);  } return 0; }" 0
try "int main(){ int array[4]; array[0] = 10; array[1] = 20; array[2] = 30; array[3] = 40; int sum; sum = array[0] + array[1] + array[2] + array[3]; print(1, sum); return sum; }" 100
try "int main(){ int array[4]; array[0] = 10; array[1] = 20; array[2] = 30; array[3] = 40; int sum; sum = array[0] + array[1] + array[2] + array[3]; print(1, sum); return sum / 100; }" 1
try "int main(){ int array[10]; int idx; for(idx = 0; idx < 10; idx = idx + 1){ array[idx] = idx * 10; } int sum; sum = 0; for(idx = 0; idx < 10; idx = idx + 1){ sum = sum + array[idx]; } return sum / 10; }" 45
try "int main(){ int array[2]; array[0] = 50; 1[array] = 20; return 0[array] - array[1]; }" 30
try "int main(){ int memo[10]; memo[0] = 1; memo[1] = 1;int idx; for(idx = 2; idx < 10; idx = idx + 1){ memo[idx] = memo[idx-1] + memo[idx-2]; } return memo[9]; }" 55

## global variable
echo -e "\e[1m\nglobal variable\e[m"
try "int a; int b; int c; int main(){ a = 10; b = 20; c = 30; return a + b - c; }" 0
try "int cnt; int main(){ cnt = 0; int idx; for(idx = 0; idx < 20; idx = idx + 1) { countup(); } return cnt; } int countup(){ cnt = cnt + 1; }" 20

## char
echo -e "\e[1m\nchar\e[m"
try "int main(){ char a; char b; a = 1; b = 2; return a + b; }" 3
try "int main(){ char a; return sizeof(a); }" 1
try "int main(){ char *a; return sizeof(a); }" 8
try "int main(){ char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }" 3
try "int main(){ char a; char b; a = 128; b = -129; return a + b + 1; }" 0

## str
echo -e "\e[1m\nstr\e[m"
try "int main(){ \"hello world\"; }" 0
try "int main(){ \"yncc : c-compiler\"; }" 0
try "int main(){ char *x; x = \"helloworld %d %d\\n\"; printf(x, 1, 2); }" 0
try "int main(){ printf(\"Hello World\!\\n\"); }" 0
try "int main(){ char *str; str = \"hello world\"; return str[0] + str[1]; }" 205

## comment
echo -e "\e[1m\ncomment\e[m"
try "int main(){ /* abcdefghijklmn */ int a; a = 20; return a - 20; }" 0

## init global variable
echo -e "\e[1m\ninit global variable\e[m"
try "int a = 10; int b = 20; int c = 30; int sum; int main(){ func(40); return sum; } int func(int d){ sum = a + b + c + d; }" 100
try "int a; int *b = &a; char c[] = \"hello world\"; char *d = c+3; int main(){ a = 20; if(*b != 20) return 0; int idx; for(idx = 3; c[idx] != 0; idx = idx+1){ if(c[idx] != d[idx-3]) return 1; } return 0; }" 0
try "int a[] = {1, 2, 3, 4, 5}; int b[5] = {1, 2}; int c[5]; int main(){ int idx; for(idx = 0; idx < 5; idx = idx+1) printf(\"a = %d, b = %d, c = %d\\n\", a[idx], b[idx], c[idx]); }" 0
try "int memo[10]; int main(){ int idx; for(idx = 1; idx < 10; idx = idx+1){ printf(\"%d\\n\", fib(idx)); }} int fib(int num){ if(num <= 2){ return 1; } if(memo[num] == 0){ memo[num] = fib(num-1) + fib(num-2); } return memo[num]; }" 0

## init local variable
echo -e "\e[1m\ninit local variable\e[m"
try "int main(){ int a = 10; int b = 20; int c = 30; return a+b+c; }" 60
try "int main(){ char *msg = \"hello %s!\\n\"; char *name = \"Yuta1004\"; printf(msg, name); }" 0
try "int main(){ int x = 20; int *y = &x; int **z = &y; return **z; }" 20
try "int main(){ int num[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; int sum = 0; int idx = 0; for(; idx < 10; idx = idx + 1) sum = sum + num[idx]; return sum; }" 55

## scope
echo -e "\e[1m\nscope\e[m"
try "int main() { int a = 10; { int a = 20; { int a = 30; printf(\"%d\\n\", a); } printf(\"%d\\n\", a); } printf(\"%d\\m\", a); }" 0

## for-ext
echo -e "\e[1m\nfor-ext\e[m"
try "int main() { int sum = 0; for(int a = 1; a <= 10; a = a+1) sum = sum+a; return sum; }" 55
try "int main() { int sum = 0; for(int i = 1; i <= 5; i = i+1) for(int j = 1; j <= 5; j = j+1) sum = sum+j; return sum; }" 75
try "int main() { int sum = 0; for(int i = 1; i <= 10; i = i+1) { for(int j = 1; j <= 10; j = j+1) { int sum = 1204 + j; } sum = sum+i; } return sum; }" 55

## comp-assign
echo -e "\e[1m\ncomp-assign\e[m"
try "int main() { int a = 0; a += 10; a -= 5; return a; }" 5
try "int main() { int a = 10; a *= 5; a /= 10; return a; }" 5
try "int main() { int sum = 0; for(int n = 1; n <= 10; n += 1) { sum += n; } return sum; }" 55

## inc
echo -e "\e[1m\ninc\e[m"
try "int main() { int a = 0; ++ a; ++ a; ++ a; return a; }" 3
try "int main() { int a = 0; int b = ++ a; return a+b; }" 2
try "int main() { int i; for(i = 0; i < 10; ++ i); return i; }" 10
try "int main() { int a = 10; int b = a ++; return a+b; }" 21
try "int main() { int a = 0; ++ a; a ++; ++ a; a ++; a ++; return a; }" 5

## dec
echo -e "\e[1m\ndec\e[m"
try "int main() { int a = 10; -- a; -- a; -- a; return a; }" 7
try "int main() { int a = 10; int b = -- a; return a+b; }" 18
try "int main() { int i; for(i = 10; i > 0; -- i); return i; }" 0
try "int main() { int a = 10; int b = a --; return a+b; }" 19
try "int main() { int a = 5; -- a; a --; -- a; a --; a --; return a; }" 0

## break
echo -e "\e[1m\nbreak\e[m"
try "int main() { int n = 0; for(; n < 10; ++ n) if(n == 5) break; return n; }" 5
try "int main() { int sum = 0; for(int i = 0; i < 5; ++ i) for(int j = 0; j < 5; ++ j) { if(j == 2) break; sum += j; } return sum; }" 5
try "int main() { int n = 1; int sum = 0; while(1) { sum += n; ++ n; if(sum >= 55) break; } return sum; }" 55

## continue
echo -e "\e[1m\ncontinue\e[m"
try "int main() { int sum = 0; for(int n = 1; n <= 10; ++ n) { if(n == 5) continue; sum += n; } return sum; }" 50
try "int main() { int sum = 0; for(int i = 0; i < 5; ++ i) for(int j = 0; j < 5; ++ j) { if(j == 2) continue; sum += j; } return sum; }" 40
try "int main() { int sum = 1; int loop_cnt = 0; while(sum < 100) { ++ loop_cnt;  if(sum == 50) { sum = 80;  continue; } else { ++ sum; } } return loop_cnt; }" 70

## struct
echo -e "\e[1m\nstruct\e[m"
try "int main() { struct User { char *name; int age; }; }" 0
try "int main() { struct User { char *name; int age; }; struct User user; user.name = \"nakagamiyuta\"; user.age = 18; printf(\"--- UserInfo ---\\n\"); printf(\"- Name: %s\\n\", user.name); printf(\"- Age: %d\\n\", user.age); }" 0
try "int main() { struct TestStruct { int a; }; { struct TestStruct { int e; }; struct TestStruct test; test.e = 10; if(test.e != 10) return 1; } struct TestStruct test; test.a = 50; if(test.a != 50) return 1; return 0; }" 0
try "int main() { struct Item { int id; }; struct Item items[10]; for(int idx = 0; idx < sizeof(items)/sizeof(items[0]); ++ idx) { items[idx].id = idx+1; } int sum = 0; for(int idx = 0; idx < 10; idx ++) { sum += items[idx].id; }; return sum; }" 55
try "struct Data { int a; int b; int *c; }; int print(struct Data *d) { puts(\"Data\"); printf(\"- a : %d\\n\", d->a); printf(\"- b : %d\\n\", d->b); printf(\"- c : %d (%p)\\n\", *(d->c), d->c); } int main() { int num = 30; struct Data data1; data1.a = 10; data1.b = 20; data1.c = &num; print(&data1); }" 0
try "int assert(int a, int b) { if(a != b) exit(1); } int main() { struct Data { int a; int b; int *c; }; int num = 30; struct Data d; struct Data *pd; pd = &d; d.a = 10; d.b = 20; d.c = &num; assert(pd->a, 10); assert(pd->b, 20); assert(*(pd->c), 30); }" 0
try "int assert(int a, int b) { if(a != b) exit(1); } int main() { struct DataA { int a; int b; int c; }; struct DataB {  struct DataA a; int b; }; struct DataC {  struct DataB a; int b; }; struct DataC data; data.a.a.a = 10; data.a.a.b = 20; data.a.a.c = 30; data.a.b = 40; data.b = 50; assert(data.a.a.a, 10); assert(data.a.a.b, 20); assert(data.a.a.c, 30); assert(data.a.b, 40); assert(data.b, 50); }" 0
try "int assert(int a, int b) { if(a != b) exit(1); } int main(){ struct DataA { int a; int b; }; struct DataB { struct DataA a; int b; }; struct DataB data[10]; for(int idx = 0; idx < 10; ++ idx) { data[idx].a.a = idx*10+1; data[idx].a.b = idx*10+2; data[idx].b = idx*10+3; } for(int idx = 0; idx < 10; ++ idx) { assert(data[idx].a.a, idx*10+1); assert(data[idx].a.b, idx*10+2); assert(data[idx].b, idx*10+3); } }" 0
try "struct DataA { int a; int b; }; struct DataB { struct DataA a; int b; }; int assert(int a, int b) { if(a != b) exit(1); } int check(struct DataB *data, int idx) { assert(data->a.a, idx*10+1); assert(data->a.b, idx*10+2); assert(data->b, idx*10+3); } int main() { struct DataB data[10]; for(int idx = 0; idx < 10; ++ idx) { data[idx].a.a = idx*10+1; data[idx].a.b = idx*10+2; data[idx].b = idx*10+3; check(&data[idx], idx); } }" 0

## str-ext
echo -e "\e[1m\nstr-ext\e[m"
try "int main() { printf(\"Name: \\\"yncc\\\"\\n\"); }" 0
try "int main() { int label = 1204; printf(\".str%d:\\n\", label); }" 0
try "int main() { char *str = \"yncc\"; printf(\"\\t\\t.string \\\"%s\\\\0\\\"\\n\", str); }" 0

## do-while
echo -e "\e[1m\ndo-while\e[m"
try "int main() { int sum = 0; do { sum ++; } while(0); return sum; }" 1
try "int comp_cnt = 0; int compare(int num) { ++ comp_cnt; return num < 10; } int main() { int sum = 0; do { sum ++; } while(compare(sum)); return comp_cnt; }" 10

## bit
echo -e "\e[1m\nbit\e[m"
try "int assert(int num_a, int num_b) { if(num_a != num_b) exit(1); } int main() { assert(0 & 0, 0); assert(0 & 1, 0); assert(1 & 0, 0); assert(1 & 1, 1); return 0; }" 0
try "int assert(int num_a, int num_b) { if(num_a != num_b) exit(1); } int main() { assert(0 | 0, 0); assert(0 | 1, 1); assert(1 | 0, 1); assert(1 | 1, 1); return 0; }" 0
try "int assert(int num_a, int num_b) { if(num_a != num_b) exit(1); } int main() { assert(0 ^ 0, 0); assert(0 ^ 1, 1); assert(1 ^ 0, 1); assert(1 ^ 1, 0); return 0; }" 0
try "int assert(int num_a, int num_b) { if(num_a != num_b) exit(1); } int main() { int a = 1; int b = 0; b |= a; assert(a, 1); assert(b, 1); a &= b; assert(a, 1); assert(b, 1); a ^= b; assert(a, 0); assert(b, 1); b &= a; assert(a, 0); assert(b, 0); }" 0

## comp
echo -e "\e[1m\ncomp\e[m"
try "int main() { if(0 && 0) exit(1); if(0 && 1) exit(1); if(1 && 0) exit(1); if(1 && 1){} else exit(1); if(100 && 200){} else exit(1); if(12 && 0) exit(1); exit(0); }" 0
try "int main() { if(0 || 0) exit(1); if(0 || 1){} else exit(1); if(1 || 0){} else exit(1); if(1 || 1){} else exit(1); if(100 || 200){} else exit(1); if(12 || 0){} else exit(1); exit(0); }" 0



echo ""
exit 0
