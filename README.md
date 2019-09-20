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
./yncc "int main(){ return 0; }" > tmp.s
gcc -o tmp tmp.s
./tmp
echo $?

// Test
cd compiler
make test
```
