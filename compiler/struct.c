#include <stdio.h>
#include <string.h>
#include "yncc.h"
#include "vector.h"

// Struct構造体生成
Struct *new_struct(char *tag, int dtype) {
    Struct *_struct = calloc(1, sizeof(Struct));
    _struct->tag = tag;
    _struct->members = vec_new(10);
    _struct->names = vec_new(10);
    _struct->dtype = dtype;
    return _struct;
}

// 構造体定義
// "{" (type ident ("," type ident)*)? "}"
// structキーワード, タグ名は既にパースされた前提
bool def_struct(int type, char *tag) {
    // {
    if(!consume("{"))
        return false;

    // (type ident ("," type ident)*)?
    Struct *_struct = new_struct(tag, type);
    while(true) {
        // type ident
        Type *type = read_type();
        if(!type) break;
        Token *ident = expect_ident();
        if(!ident) break;
        expect(";");

        // add vec
        char *name = malloc(ident->len);
        strncpy(name, ident->str, ident->len);
        name[ident->len] = 0;
        vec_push(_struct->members, type);
        vec_push(_struct->names, name);
    }

    // struct_list追加
    vec_push(struct_def_list, _struct);
    expect("}");
    return true;
}