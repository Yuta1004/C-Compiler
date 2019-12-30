#include "yncc.h"
#include <stdio.h>
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
    Type *type = new_type(INT);
    Node *node = calloc(1, sizeof(Node));
    node->type = type;
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 変数ノード生成
Node *new_var_node(Var *var) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->name = var->name;
    node->type = var->type;
    node->offset = var->offset;
    return node;
}

// Noneノード生成
Node *new_none_node() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NONE;
    return node;
}