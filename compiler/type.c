#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yncc.h"

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
    case STRUCT:
        return type->bytesize;
    default:
        return 1;
    }
}

// type_ptrに型を設定する
void define_type(Type **type_ptr, int ty){
    Type *tmp = new_type(ty);
    *type_ptr = tmp;
}

// type_ptrに型をコピーする
void copy_type(Type **dst, Type *src) {
    *dst = new_type(NONE);
    if(src != NULL)
        memmove(*dst, src, sizeof(Type));
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
    Token *bef_token = token;
    Type *b_type = new_type(NONE);

    // ("int" | "char" | "struct")
    if(consume_kind(TOKEN_INT)) {
        b_type->ty = INT;
        b_type->bytesize = 4;
    } else if(consume_kind(TOKEN_CHAR)) {
        b_type->ty = CHAR;
        b_type->bytesize = 1;
    } else if(consume_kind(TOKEN_STRUCT)) {
        Token *ident = expect_ident();
        if(!consume("{")) {     // 構造体定義か調べる
            b_type->ty = STRUCT;
            b_type->tag = ident->str;
            b_type->len = ident->len;
            b_type->bytesize = get_struct_size(ident->str, ident->len);
            if(b_type->bytesize < 0)
                error_at(ident->str, "定義されていない構造体です");
        } else {
            token = bef_token;
            return NULL;
        }
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