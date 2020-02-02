#include "../yncc.h"
#include "parse.h"

// add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();
    while(true) {
        if(consume("+")) {
            node = new_node_lr(ND_ADD, node, mul());
        } else if(consume("-")) {
            node = new_node_lr(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// mul = unary ("*" unary | "-" unary)*
Node *mul(){
    Node *node = unary();
    while(true) {
        if(consume("*")) {
            node = new_node_lr(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node_lr(ND_DIV, node, unary());
        } else if(consume("%")) {
            node = new_node_lr(ND_DIV_REMAIN, node, unary());
        } else {
            return node;
        }
    }
}