#include <stdlib.h>
#include "yncc.h"
#include <stdio.h>

// Type構造体生成
Type *new_type(int ty) {
    Type *type = calloc(1, sizeof(Type));
    type->ty = ty;
    type->bytesize = type_to_size(type);
    return type;
}

// 型のサイズを求める
int type_to_size(Type *type) {
    if(type == NULL) return 1;
    switch(type->ty){
    case CHAR:
        return 1;
    case INT:
        return 4;
    case PTR:
    case ARRAY:
        return 8;
    default:
        return 1;
    }
}

// type_ptrに型を設定する
void define_type(Type **type_ptr, int ty){
    Type *tmp = new_type(ty);
    *type_ptr = tmp;
}

// 大きい方の型の種類を返す
Type *max_type(Type *a, Type *b){
    if(a == NULL && b == NULL)
        return new_type(-1);
    if(a == NULL)
        return b;
    if(b == NULL)
        return a;

    if(type_to_size(a) > type_to_size(b)){
        a->bytesize = type_to_size(a);  // 配列サイズ->型のサイズ
        return a;
    }
    b->bytesize = type_to_size(b);  // 配列サイズ->型サイズ
    return b;
}

// 型を読む
// type = ("int" | "char") "*"*
Type *read_type() {
    Type *b_type = new_type(NONE);

    // ("int" | "char")
    if(consume_kind(TOKEN_INT)) {
        b_type->ty = INT;
        b_type->bytesize = 4;
    } else if(consume_kind(TOKEN_CHAR)) {
        b_type->ty = CHAR;
        b_type->bytesize = 1;
    } else {
        return NULL;
    }

    // "*"*
    Type *lead_type = b_type;
    while(consume("*")) {
        Type *ptr_type = new_type(PTR);
        ptr_type->ptr_to = lead_type;
        lead_type = ptr_type;
    }
    return lead_type;
}

// ポインタを辿ってベースとなる型を探す
Type *get_base_type(Type *type) {
    Type *now_pos = type;
    while(true) {
        if(now_pos->ptr_to == NULL) {
            return now_pos;
        }
        now_pos = now_pos->ptr_to;
    }
}