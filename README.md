# C-compiler

![](https://github.com/Yuta1004/yncc/workflows/Compiler%2DTest/badge.svg)

## Reference

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)

## Dir

- machine-and-assembly
    - 機械語とアセンブリ

- compiler
    - コンパイラ本体

## Commands

```
// Run
cd compiler
make
./yncc "return 0;"
echo $?

// Test
cd compiler
make test
```
