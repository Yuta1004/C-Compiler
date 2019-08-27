#include <stdio.h>
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
    printf("main:\n");

    // ローカル変数領域確保
    printf("        push rbp\n");
    printf("        mov rbp, rsp\n");
    printf("        add rsp, %d\n", 8*26);

    // アセンブリ出力
    for(int idx = 0; code[idx] != NULL; ++ idx){
        gen_asm(code[idx]);
        printf("        pop rax\n");    // 式の最終的な値を取り出す
    }

    // フッター
    printf("        mov rsp, rbp\n");
    printf("        pop rbp\n");
    printf("        ret\n");
    return 0;
}
