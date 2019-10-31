#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "yncc.h"


// 変数検索
Var *find_var(Token *request){
    // 検索
    Vector *var_list[] = {locals, globals};
    for(int idx = 0; idx < 2; ++ idx) {
        for(int v_idx = 0; v_idx < var_list[idx]->len; ++ v_idx){
            Var *var = (Var*)vec_get(var_list[idx], v_idx);
            if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
                return var;
            }
        }
    }

    char *name = (char*)calloc(request->len+1, sizeof(char));
    strncpy(name, request->str, request->len);
    error("[ERROR] 定義されていない変数です -> %s", name);
}

// 変数登録
Var *regist_var(int var_type){
    // 型
    Type *type = read_type();
    if(!type) return NULL;
    Type *base_type = get_base_type(type);

    // 変数名
    Var *var = calloc(1, sizeof(Var));
    Token *var_name = consume_ident();
    var->var_type = var_type;
    var->type = type;
    var->len = var_name->len;
    var->offset = (sum_offset += 8);
    var->name = (char*)calloc(var_name->len+1, sizeof(char));
    strncpy(var->name, var_name->str, var_name->len);
    if(var_type == LOCAL)
        vec_push(locals, var);
    if(var_type == GLOBAL)
        vec_push(globals, var);

    // LOCAL -> "[" array_size "]"
    // GLOBAL -> "[" array_size? "]"
    switch(var_type){
    case LOCAL:
        if(consume("[")) {
            size_t size = expect_number();
            define_type(&base_type->ptr_to, base_type->ty);
            base_type->ty = ARRAY;
            base_type->size = size;
            var->offset = (sum_offset += (size*8) - 8);
            expect("]");
            if(size <= 0) {
                error("[ERROR] 長さが0以下の配列は定義できません");
            }
        }

    case GLOBAL:
        if(consume("[")) {
            Token *size = consume_number();
            define_type(&base_type->ptr_to, base_type->ty);
            base_type->ty = ARRAY;
            if(size)
                base_type->size = size->val;
            expect("]");
            if(size != 0 && size->val <= 0) {
                error("[ERROR] 長さが0以下の配列は定義できません");
            }
        }
    }
    return var;
}