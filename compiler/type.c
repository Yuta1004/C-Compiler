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