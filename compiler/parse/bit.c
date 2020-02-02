#include "../yncc.h"
#include "parse.h"

// bit_and = equality ("&" | bit_and)*
Node *bit_and(){
    Node *node = equality();
    while(consume("&"))
        node = new_node_lr(ND_BIT_AND, node, bit_and());
    return node;
}

// bit_xor = bit_and ("^" | bit_xor)*
Node *bit_xor() {
    Node *node = bit_and();
    while(consume("^"))
        node = new_node_lr(ND_BIT_XOR, node, bit_xor());
    return node;
}

// bit_or = bit_xor ("|" bit_or)*
Node *bit_or() {
    Node *node = bit_xor();
    while(consume("|"))
        node = new_node_lr(ND_BIT_OR, node, bit_or());
    return node;
}

// bit_shift = add ("<<" add | ">>" add)*
Node *bit_shift() {
    Node *node = add();
    while(true) {
        if(consume("<<")) {
            node = new_node_lr(ND_BIT_SHIFT_L, node, add());
        } else if(consume(">>")) {
            node = new_node_lr(ND_BIT_SHIFT_R, node, add());
        } else {
            return node;
        }
    }
}
