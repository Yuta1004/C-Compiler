#include <stdio.h>
#include <string.h>
#include "yncc.h"
#include "vector.h"

// TODO: 構造体初期化, コピー

// Struct構造体生成
Struct *new_struct(int var_type, char *tag, int len) {
    Struct *_struct = calloc(1, sizeof(Struct));
    _struct->tag = tag;
    _struct->len = len;
    _struct->members = vec_new(10);
    _struct->names = vec_new(10);
    _struct->var_type = var_type;
    _struct->scope_id = scope_id;
    return _struct;
}

// 構造体定義
// "{" (type ident ("," type ident)*)? "}"
// structキーワード, タグ名は既にパースされた前提
bool def_struct(int var_type, char *tag, int len) {
    // {
    if(!consume("{"))
        return false;

    // (type ident ("," type ident)*)?
    int max_alignment = 0;
    Struct *_struct = new_struct(var_type, tag, len);
    while(true) {
        // type ident
        Type *type = read_type();
        if(!type) break;
        Token *ident = expect_ident();
        if(!ident) break;
        if(max_alignment < type->bytesize)
            max_alignment = type->bytesize;

        // "[" num "]"
        if(consume("[")) {
            size_t size = expect_number();
            copy_type(&type->ptr_to, type);
            type->ty = ARRAY;
            type->bytesize *= size;
            expect("]");
        }

        // add vec
        char *name = malloc(ident->len);
        strncpy(name, ident->str, ident->len);
        name[ident->len] = 0;
        vec_push(_struct->members, type);
        vec_push(_struct->names, name);
        expect(";");
    }

    // アラインメント
    // (padding) [1] (padding) [2] ...
    int offset = 0;
    for(int idx = 0; idx < _struct->members->len; ++ idx) {
        Type *member = vec_get(_struct->members, idx);
        member->padsize = (max_alignment-offset%max_alignment) % max_alignment;
        offset += member->padsize + member->bytesize;
    }
    _struct->bytesize = offset + (max_alignment-offset%max_alignment) % max_alignment;

    // struct_list追加
    vec_push(struct_def_list, _struct);
    expect("}");
    return true;
}

// 構造体のタグからそのサイズを返す
int get_struct_size(char *tag, int len) {
    for(int idx = 0; idx < struct_def_list->len; ++ idx) {
        Struct *_struct = vec_get(struct_def_list, idx);
        if(_strncmp(_struct->tag, tag, _struct->len, len))
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
    Var *hit_var = NULL;
    for(int idx = 0; idx < struct_def_list->len; ++ idx) {
        // 目当ての構造体か
        Struct *_struct = vec_get(struct_def_list, idx);
        if( !_strncmp(_struct->tag, tag, _struct->len, tag_len) || // タグ一致
            vec_find(man_scope, (void*)(long)(_struct->scope_id)) == -1 || // スコープ
            (hit_var != NULL && hit_var->scope_id >= _struct->scope_id)    // ネストの深さ
        ) { continue; };

        // メンバ
        int offset = 0;
        for(int m_idx = 0; m_idx < _struct->members->len; ++ m_idx) {
            Type *member_type = vec_get(_struct->members, m_idx);
            char *member_name = vec_get(_struct->names, m_idx);
            offset += member_type->padsize;
            if(_strncmp(member_name, member_n, strlen(member_name), mn_len))
                return member_to_var(member_n, mn_len, member_type, offset);
            offset += member_type->bytesize;
        }
    }
    return NULL;
}