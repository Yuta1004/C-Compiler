#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "yncc.h"


// 変数検索
Var *find_var(Token *request){
    // 検索
    Var *var_list[] = {locals, globals};
    for(int idx = 0; idx < 2; ++ idx) {
        for(Var *var = var_list[idx]; var; var = var->next){
            if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
                return var;
            }
        }
    }

    char *name = (char*)malloc(request->len*sizeof(char));
    strncpy(name, request->str, request->len);
    error("[ERROR] 定義されていない変数です -> %s", name);
}

// 変数登録
Var *regist_var(int var_type){
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
    Var *var = calloc(1, sizeof(Var));
    Token *var_name = consume_ident();
    var->var_type = var_type;
    var->type = type;
    var->len = var_name->len;
    var->offset = locals->offset + 8;
    var->name = (char*)malloc(var_name->len*sizeof(char));
    strncpy(var->name, var_name->str, var_name->len);
    if(var_type == LOCAL) {  var->next = locals; locals = var; }
    if(var_type == GLOBAL) { var->next = globals; globals = var; }

    // "[" array_size "]"
    if(consume("[")) {
        size_t size = expect_number();
        int_type->ty = ARRAY;
        int_type->size = size;
        var->offset = locals->offset - 8 + (8 * size);
        expect("]");
        if(size <= 0) {
            error("[ERROR] 長さが0以下の配列は定義できません");
        }
    }
    return var;
}