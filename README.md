# C-compiler

![](https://github.com/Yuta1004/yncc/workflows/Compiler%2DTest/badge.svg)

## Reference

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)

## Dir

- compiler
    - コンパイラ本体

## Commands

```
// Run
cd compiler
make
vim program.c 
./yncc program.c > tmp.s
gcc -o tmp tmp.s
./tmp
echo $?

// Test(C)
cd compiler
make test

// Test(SH)
cd compiler
make test-sh
```
