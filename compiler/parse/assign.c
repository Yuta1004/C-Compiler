#include "../yncc.h"
#include "parse.h"

// assign = log_or ("=" assign)?
Node *assign(){
    Node *node = log_or();
    if(consume("=")){
        node = new_node_lr(ND_ASSIGN, node, assign());
    } else if(consume("+=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_ADD, node, assign()));
    } else if(consume("-=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_SUB, node, assign()));
    } else if(consume("*=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_MUL, node, assign()));
    } else if(consume("/=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_DIV, node, assign()));
    } else if(consume("&=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_BIT_AND, node, assign()));
    } else if(consume("|=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_BIT_OR, node, assign()));
    } else if(consume("^=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_BIT_XOR, node, assign()));
    } else if(consume("<<=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_BIT_SHIFT_L, node, assign()));
    } else if(consume(">>=")){
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(ND_BIT_SHIFT_R, node, assign()));
    }
    return node;
}