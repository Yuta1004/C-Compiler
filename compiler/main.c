#include <stdio.h>
#include <stdlib.h>
#include "yncc.h"
#include "vector.h"
#include "file.h"

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "[ERROR] 引数が少なすぎます！\n");
        return 0;
    }

    // プログラムファイル読み込み
    program_body = read_file(argv[1]);

    // トークナイズ
    str_vec = vec_new(30);
    token = tokenize(program_body);

    // 構文木生成
    program();

    // ヘッダー
    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");
    printf(".data\n");

    // 文字列<ヘッダー>
    for(int idx = 0; idx < str_vec->len; ++ idx) {
        char *str = (char*)vec_get(str_vec, idx);
        if(str == NULL) break;
        printf(".str%d:\n", label);
        printf("\t\t.string \"%s\"\n", str);
        ++ label;
    }
    printf("\n");

    // グローバル変数<ヘッダー>
    for(Var *gvar = globals; gvar; gvar = gvar->next) {
        Node *gvar_node = calloc(1, sizeof(Node));
        gvar_node->name = gvar->name;
        gen_asm(new_node(ND_INIT_GVAR, gvar_node, gvar->init_expr));
    }
    printf("\n");

    // アセンブリ出力
    printf(".text\n");
    label = 0;
    for(int idx = 0; code[idx] != (Node*)-1; ++ idx){
        gen_asm(code[idx]);
    }
    vec_free(str_vec);
    return 0;
}
