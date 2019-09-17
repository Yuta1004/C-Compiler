#include <stdio.h>
#include <stdlib.h>
#include "yncc.h"

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "[ERROR] 引数が少なすぎます！\n");
        return 0;
    }

    // トークナイズ
    user_input = argv[1];
    token = tokenize(argv[1]);

    // 構文木生成
    program();

    // ヘッダー
    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");

    // アセンブリ出力
    label = 0;
    for(int idx = 0; code[idx] != NULL; ++ idx){
        gen_asm(code[idx]);
    }
    return 0;
}
