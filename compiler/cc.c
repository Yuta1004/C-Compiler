#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "[ERROR] 引数が少なすぎます！\n");
        return 0;
    }

    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");
    printf("main:\n");
    printf("        mov rax, %d\n", atoi(argv[1]));
    printf("        ret\n");
    printf("\n");
}