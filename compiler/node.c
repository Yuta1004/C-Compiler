#include "yncc.h"
#include <stdlib.h>


// ノード生成
Node *new_node(NodeKind kind, Node *left, Node *right){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

// 数字ノード生成
Node *new_num_node(int val){
    Type *type = calloc(1, sizeof(Type));
    type->ty = INT;
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// グローバル変数定義ノード生成
Node *new_def_gvar_node(GVar *gvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEFGVAR;
    node->name = gvar->name;
    node->type = gvar->type;
    return node;
}