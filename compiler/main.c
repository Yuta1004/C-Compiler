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

    // スコープ管理
    scope_id = 0;
    scope_sum_id = 0;
    man_scope = vec_new(10);
    vec_push(man_scope, 0);

    // プログラムファイル読み込み
    program_body = read_file(argv[1]);

    // トークナイズ
    str_vec = vec_new(30);
    token = tokenize(program_body);

    // 構文木生成
    codes = vec_new(50);
    globals = vec_new(10);
    locals = vec_new(10);
    globals_struct = vec_new(10);
    locals_struct = vec_new(10);
    struct_def_list = vec_new(10);
    man_typedef = vec_new(5);
    program();

    // ヘッダー
    outtxt(".intel_syntax   noprefix\n");
    outtxt(".global         main\n");
    outtxt("\n");

    // 文字列<ヘッダー>
    outtxt(".section .rodata\n");
    for(int idx = 0; idx < str_vec->len; ++ idx) {
        char *str = (char*)vec_get(str_vec, idx);
        if(str == NULL) break;
        printf(".str%d:\n", label);
        printf("\t\t.string \"%s\\0\"\n", str);
        ++ label;
    }
    outtxt("\n");

    // グローバル変数<ヘッダー>
    outtxt(".data\n");
    for(int idx = 0; idx < globals->len; ++ idx) {
        Var *gvar = (Var*)vec_get(globals, idx);
        Node *gvar_node = new_var_node(gvar);
        gen_asm(new_node_lr(ND_INIT_GVAR, gvar_node, gvar->init_expr));
    }
    outtxt("\n");

    // アセンブリ出力
    outtxt(".text\n");
    for(int idx = 0; idx < codes->len; ++ idx){
        gen_asm(vec_get(codes, idx));
    }

    // 後処理
    vec_free(locals);
    vec_free(globals);
    vec_free(locals_struct);
    vec_free(globals_struct);
    vec_free(struct_def_list);
    vec_free(str_vec);
    vec_free(man_scope);
    vec_free(man_typedef);
    return 0;
}
