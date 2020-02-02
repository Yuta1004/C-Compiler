#include "../yncc.h"
#include "parse.h"

// bit_xor = bit_and ("^" | bit_xor)*
Node *bit_xor() {
    Node *node = bit_and();
    while(consume("^"))
        node = new_node_lr(ND_BIT_XOR, node, bit_xor());
    return node;
}
