#include <stdlib.h>

#include "../yncc.h"
#include "parse.h"

// unary = "sizeof" unary | ("+" | "-")? accessor | ("*" | "&") unary | unary "[" unary "]"  |
//         ("++" | "--" | "!" | "~") accessor
Node *unary(){
    if(consume_kind(TOKEN_SIZEOF)) {
        Node *node = unary();
        int bytesize = node->type->bytesize;
        free(node);
        return new_num_node(bytesize);
    }

    if(consume("*")) {
        Node *node = new_node_lr(ND_DEREF, unary(), NULL);
        node->type = node->left->type->ptr_to;
        return node;
    }

    if(consume("&")) {
        Node *node = new_node_lr(ND_ADDR, unary(), NULL);
        define_type(&node->type, PTR);
        define_type(&node->type->ptr_to, PTR);
        return node;
    }

    if(consume("-"))
        return new_node_lr(ND_SUB, new_num_node(0), accessor());

    if(consume("++")) // 前置
        return new_node_lr(ND_PRE_INC, accessor(), NULL);

    if(consume("--")) // 前置
        return new_node_lr(ND_PRE_DEC, accessor(), NULL);

    if(consume("~"))  // ビット単位の論理否定
        return new_node_lr(ND_BIT_NOT, accessor(), NULL);

    if(consume("!"))
        return new_node_lr(ND_EQ, accessor(), new_num_node(0));

    return accessor();
}