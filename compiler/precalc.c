#include <stdio.h>
#include <stdlib.h>
#include "yncc.h"

char *precalc_expr(Node *root);
int precalc_expr_child(Node *node);

char *precalc(Node *root) {
    // 式
    return precalc_expr(root);
}

char *precalc_expr(Node *root) {
    // 式評価
    int size, result = precalc_expr_child(root);
    for(size = 1; result / (size * 10) > 0; ++ size);

    // 文字列にして返す
    char *ret_val = malloc((size + 1) * sizeof(char));
    sprintf(ret_val, "%d", precalc_expr_child(root));
    return ret_val;
}

int precalc_expr_child(Node *node) {
    if(node == NULL) return 0;
    int left = precalc_expr_child(node->left);
    int right = precalc_expr_child(node->right);

    switch(node->kind) {
    case ND_ADD:
        return left + right;

    case ND_SUB:
        return left - right;

    case ND_MUL:
        return left * right;

    case ND_DIV:
        return left / right;

    case ND_DIV_REMAIN:
        return left % right;

    case ND_NUM:
        return node->val;

    default:
        error("式が定数式ではありません");
    }
}