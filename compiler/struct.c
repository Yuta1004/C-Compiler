#include <stdio.h>
#include <string.h>
#include "yncc.h"
#include "vector.h"

#define _strncmp(str1, str2, str1_len, str2_len) \
    ((str1_len == str2_len) && strncmp(str1, str2, str1_len) == 0)

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

// 構造体のタグからそのサイズを返す
int get_struct_size(char *tag, int len) {
    for(int idx = 0; idx < struct_def_list->len; ++ idx) {
        Struct *_struct = vec_get(struct_def_list, idx);
        if(_strncmp(_struct->tag, tag, strlen(_struct->tag), len))
            return _struct->bytesize;
    }
    return -1;
}

// メンバに応じたVar構造体を返す
Var *member_to_var(char *name, int len, Type *type, int offset) {
    Var *var = calloc(1, sizeof(Var));
    var->name = name;
    var->len = len;
    var->type = type;
    var->offset = offset;
    return var;
}

// タグとメンバ名からVar構造体を返す, 呼び出し元で変数のoffsetをプラスする
Var *struct_get_member(char *tag, int tag_len, char *member_n, int mn_len) {
    // locals検索
    for(int idx = 0; idx < struct_def_list->len; ++ idx) {
        // タグ
        Struct *_struct = vec_get(struct_def_list, idx);
        fprintf(stderr, "%s, %s, %d %d\n", _struct->tag, tag, strlen(_struct->tag), tag_len);
        if(!_strncmp(_struct->tag, tag, strlen(_struct->tag), tag_len))
            continue;

        // メンバ
        int offset = 0;
        for(int m_idx = 0; m_idx < _struct->members->len; ++ idx) {
            Type *member_type = vec_get(_struct->members, m_idx);
            char *member_name = vec_get(_struct->names, m_idx);
            if(_strncmp(member_name, member_n, strlen(member_name), mn_len))
                return member_to_var(member_n, mn_len, member_type, offset);
            offset += member_type->bytesize + member_type->alignment;
        }
    }
}