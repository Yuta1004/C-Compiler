#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "yncc.h"


// ローカル変数検索
LVar *find_lvar(Token *request){
    // 検索
    for(LVar *var = locals; var; var = var->next){
        if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
            return var;
        }
    }

    char *name = (char*)malloc(request->len*sizeof(char));
    strncpy(name, request->str, request->len);
    error("[ERROR] 定義されていない変数です => %s\n", name);
}

// 変数登録
void *regist_var(bool is_global){
    // "int"
    Type *int_type = calloc(1, sizeof(Type));
    int_type->ty = INT;
    int_type->size = 1;
    int_type->ptr_to = NULL;
    if(!consume_kind(TOKEN_INT)) {
        return NULL;
    }

    // "*"*
    Type *type = int_type;
    while(consume("*")) {
        Type *ptr_type = calloc(1, sizeof(Type));
        ptr_type->ty = PTR;
        ptr_type->ptr_to = type;
        type = ptr_type;
    }

    // 変数名
    GVar *gvar;
    LVar *lvar;
    if(is_global) {
        gvar = calloc(1, sizeof(GVar));
        Token *var_name = consume_ident();
        gvar->type = type;
        gvar->next = globals;
        gvar->name = var_name->str;
        gvar->len = var_name->len;
        globals = gvar;
    } else {
        lvar = calloc(1, sizeof(LVar));
        Token *var_name = consume_ident();
        lvar->type = type;
        lvar->next = locals;
        lvar->name = var_name->str;
        lvar->len = var_name->len;
        lvar->offset = locals->offset + 8;
        locals = lvar;
    }

    // "[" array_size "]"
    if(consume("[")) {
        size_t size = expect_number();
        int_type->ty = ARRAY;
        int_type->size = size;
        if(!is_global) lvar->offset = locals->offset - 8 + (8 * size);
        expect("]");
        if(size <= 0) {
            error("[ERROR] 長さが0以下の配列は定義できません");
        }
    }

    if(is_global) return (void*) gvar;
    else          return (void*) lvar;
}