#include <stdio.h>


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
