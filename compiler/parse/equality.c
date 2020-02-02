#include "../yncc.h"
#include "parse.h"

// equality = relational ("==" relational | "!=" relational)*
Node *equality(){
    Node *node = relational();
    if(consume("==")) {
        node = new_node_lr(ND_EQ, node, relational());
    } else if(consume("!=")) {
        node = new_node_lr(ND_NEQ, node, relational());
    }
    return node;
}