#include "../yncc.h"
#include "parse.h"

// log_and = bit_or ("&&" | log_and)*
Node *log_and()  {
    Node *node = bit_or();
    while(consume("&&"))
        node = new_node_lr(ND_BOOL_AND, node, log_and());
    return node;
}

// log_or = log_and ("||" | log_or)*
Node *log_or() {
    Node *node = log_and();
    while(consume("||"))
        node = new_node_lr(ND_BOOL_OR, node, log_or());
    return node;
}
