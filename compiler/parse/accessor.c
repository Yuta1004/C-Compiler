#include <stdlib.h>

#include "../yncc.h"
#include "parse.h"

// accessor = primary ("++" | "--") | primary "[" (ident | num ) "]" | primary ("." | "->") ident
Node *accessor() {
    Node *node = primary();

    if(consume("++")) { // 後置
        return new_node_lr(ND_POST_INC, node, NULL);
    }

    if(consume("--")) { // 後置
        return new_node_lr(ND_POST_DEC, node, NULL);
    }

    while(true) {
        // "[" (ident | num ) "]"
        if(consume("[")) {
            Node *add = new_node_lr(ND_ADD, node, expr());
            node = new_node_lr(ND_DEREF, add, NULL);
            add->type = max_type(add->left->type->ptr_to, add->right->type->ptr_to);
            copy_type(&node->type, add->type);
            expect("]");
            continue;
        }

        // primary ("." | "->") ident
        bool arrowacs = false;
        if(consume(".") || (arrowacs=consume("->"))) {
            // メンバ名取得
            Var *member;
            Token *member_n = expect_ident();
            if(arrowacs) {
                node = new_node_lr(ND_DEREF, node, NULL);
                node->type = node->left->type->ptr_to;
            }
            member = struct_get_member(node->type->tag, node->type->len, member_n->str, member_n->len);
            if(!member)
                error_at(member_n->str, "構造体名またはメンバ名が正しくありません");

            // オフセット, 型設定
            Node *deref_node = new_node(ND_DEREF);
            deref_node->left = new_node(ND_ADD);
            deref_node->left->left = new_node_lr(ND_ADDR, node, NULL);
            deref_node->left->right = new_num_node(member->offset);
            copy_type(&deref_node->left->type, node->type);
            copy_type(&deref_node->left->type->ptr_to, member->type->ptr_to);
            copy_type(&deref_node->type, member->type);
            node = member->type->ty==ARRAY ? deref_node->left : deref_node;
            continue;
        }
        break;
    }
    return node;
}