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
    return NULL;
}

// ローカル変数検索
GVar *find_gvar(Token *request){
    // 検索
    for(GVar *var = globals; var; var = var->next){
        if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
            return var;
        }
    }
    return NULL;
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
        ptr_type->size = 1;
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
        gvar->len = var_name->len;
        gvar->name = (char*)malloc(var_name->len*sizeof(char));
        strncpy(gvar->name, var_name->str, var_name->len);
        globals = gvar;
    } else {
        lvar = calloc(1, sizeof(LVar));
        Token *var_name = consume_ident();
        lvar->type = type;
        lvar->next = locals;
        lvar->len = var_name->len;
        lvar->offset = locals->offset + 8;
        lvar->name = (char*)malloc(var_name->len*sizeof(char));
        strncpy(lvar->name, var_name->str, var_name->len);
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