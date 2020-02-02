#include "../yncc.h"
#include "parse.h"

// bit_and = equality ("&" | bit_and)*
Node *bit_and(){
    Node *node = equality();
    while(consume("&"))
        node = new_node_lr(ND_BIT_AND, node, bit_and());
    return node;
}
