#include "../yncc.h"
#include "parse.h"

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