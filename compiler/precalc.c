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

void decode_precalc_expr(char *s, Node *node) {
    if(node == NULL) return;
    char *left = malloc(10*sizeof(char));
    char *right = malloc(10*sizeof(char));
    decode_precalc_expr(left, node->left);
    decode_precalc_expr(right, node->right);

    switch(node->kind) {
    case ND_ADD:
        sprintf(s, "%s%s+%s", s, left, right);
        break;

    case ND_SUB:
        sprintf(s, "%s%s-%s", s, left, right);
        break;

    case ND_NUM:
        sprintf(s, "%s%d", s, node->val);
        break;

    case ND_ADDR:
        sprintf(s, "%s%s", s, node->left->name);
        break;

    case ND_GVAR:
    case ND_LVAR:
        sprintf(s, "%s%s", s, node->name);
        break;

    default:
        error("式のパースに失敗しました");
    }
}