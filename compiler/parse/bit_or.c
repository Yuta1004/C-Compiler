#include "../yncc.h"
#include "parse.h"

// bit_or = bit_xor ("|" bit_or)*
Node *bit_or() {
    Node *node = bit_xor();
    while(consume("|"))
        node = new_node_lr(ND_BIT_OR, node, bit_or());
    return node;
}
