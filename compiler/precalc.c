#include <stdio.h>
#include <stdlib.h>
#include "yncc.h"

int precalc_expr(Node *node) {
    if(node == NULL) return 0;
    int left = precalc_expr(node->left);
    int right = precalc_expr(node->right);

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