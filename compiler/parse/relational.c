#include "../yncc.h"
#include "parse.h"

// relational = bit_shift (">" bit_shift | ">=" bit_shift | "<" bit_shift | "<=" bit_shift)*
Node *relational(){
    Node *node = bit_shift();
    if(consume(">")) {
        node = new_node_lr(ND_UPPERL, node, bit_shift());
    } else if(consume(">=")) {
        node = new_node_lr(ND_UPPEREQL, node, bit_shift());
    } else if(consume("<")) {
        node = new_node_lr(ND_UPPERL, bit_shift(), node);
    } else if(consume("<=")) {
        node = new_node_lr(ND_UPPEREQL, bit_shift(), node);
    }
    return node;
}