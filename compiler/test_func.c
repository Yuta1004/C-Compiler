#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


int funcA(){
    printf("called funcA\n");
    return 0;
}

int funcB(int num){
    printf("called funcB %d\n", num);
    return 0;
}

int funcC(int a, int b, int c, int d){
    printf("called funcC %d\n", a+b+c+d);
    return 0;
}

int funcD(int a, int b, int c, int d, int e, int f){
    printf("called funcD %d\n", a+b+c+d+e+f);
    return 0;
}

int funcE(int a, int b, int c, int d){
    printf("called funcE => %d, %d, %d, %d\n", a, b, c, d);
    return 0;
}

int funcF(int a, int b, int c, int d, int e, int f){
    printf("called funcF => %d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
    return a + b + c + d + e + f;
}

int print(int argc, ...){
    va_list va;
    va_start(va, argc);
    for(int idx = 0; idx < argc; ++ idx) {
        printf("%d ", va_arg(va, int));
    }
    printf("\n");
    va_end(va);
    return 0;
}

int *mem_alloc(int *ptr, int argc, ...) {
    ptr = (int*)malloc(argc*sizeof(int));
    va_list va;
    va_start(va, argc);
    for(int idx = 0; idx < argc; ++ idx){
        ptr[idx] = idx;
    }
    va_end(va);
    return 0;
}

