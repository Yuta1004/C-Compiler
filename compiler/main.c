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
    Node *node_top = expr();

    // ヘッダー
    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");
    printf("main:\n");

    // アセンブリ出力
    gen_asm(node_top);

    // フッター
    printf("        pop rax\n");
    printf("        ret\n");
    return 0;
}
