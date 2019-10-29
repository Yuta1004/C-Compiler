#include <stdlib.h>
#include "yncc.h"

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

// 型を指定する
void define_type(Type **type_ptr, int ty){
    Type *tmp = calloc(1, sizeof(Type));
    tmp->ty = ty;
    *type_ptr = tmp;
}

// 大きい方の型の種類を返す
Type *max_type(Type *a, Type *b){
    if(a == NULL && b == NULL){
        Type *tmp = calloc(1, sizeof(Type));
        tmp->ty = -1;
        return tmp;
    }
    if(a == NULL) return b;
    if(b == NULL) return a;

    if(type_to_size(a) > type_to_size(b)){
        return a;
    }
    return b;
}

// 型を読む
// type = ("int" | "char") "*"*
Type *read_type() {
    Type *b_type = calloc(1, sizeof(Type));
    b_type->size = 1;

    // ("int" | "char")
    if(consume_kind(TOKEN_INT)) {
        b_type->ty = INT;
    } else if(consume_kind(TOKEN_CHAR)) {
        b_type->ty = CHAR;
    } else {
        return NULL;
    }

    // "*"*
    Type *lead_type = b_type;
    while(consume("*")) {
        Type *ptr_type = calloc(1, sizeof(Type));
        ptr_type->ty = PTR;
        ptr_type->ptr_to = lead_type;
        ptr_type->size = 1;
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