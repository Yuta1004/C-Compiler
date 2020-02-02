#include "../yncc.h"
#include "node.h"

// ノード生成
Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// ノード生成(left, right)
Node *new_node_lr(NodeKind kind, Node *left, Node *right){
    Node *node = new_node(kind);
    node->kind = kind;
    node->left = left;
    node->right = right;
    node->type = max_type(left?left->type:NULL, right?right->type:NULL);
    return node;
}

// 数字ノード生成
Node *new_num_node(int val){
    Type *type = new_type(INT);
    Node *node = new_node(ND_NUM);
    node->type = type;
    node->val = val;
    return node;
}

// 変数ノード生成
Node *new_var_node(Var *var) {
    Node *node = new_node(ND_LVAR);
    node->name = var->name;
    node->type = var->type;
    node->offset = var->offset;
    return node;
}

// Noneノード生成
Node *new_none_node() {
    return new_node(NONE);
}
