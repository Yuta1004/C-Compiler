#include <stdio.h>
#include <string.h>
#include "yncc.h"
#include "vector.h"

// Struct構造体生成
Struct *new_struct(char *tag) {
    Struct *_struct = calloc(1, sizeof(Struct));
    _struct->tag = tag;
    _struct->members = vec_new(10);
    _struct->names = vec_new(10);
    return _struct;
}

// 構造体定義
// "{" (type ident ("," type ident)*)? "}"
// structキーワード, タグ名は既にパースされた前提
bool regist_struct(int type, char *tag) {
    // {
    if(!consume("{"))
        return false;

    // (type ident ("," type ident)*)?
    Struct *_struct = new_struct(tag);
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
    if(type == LOCAL)
        vec_push(locals_struct, _struct);
    else
        vec_push(globals_struct, _struct);
    expect("}");
    return true;
}