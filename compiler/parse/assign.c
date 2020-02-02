#include "../yncc.h"
#include "parse.h"

#define consume_assign_lr(str, type) \
    if(consume(str)) \
        node = new_node_lr(ND_ASSIGN, node, new_node_lr(type, node, assign()));

// assign = log_or ("=" assign)?
Node *assign(){
    Node *node = log_or();
    if(consume("="))
        node = new_node_lr(ND_ASSIGN, node, assign());
    consume_assign_lr("+=", ND_ADD);
    consume_assign_lr("-=", ND_SUB);
    consume_assign_lr("*=", ND_MUL);
    consume_assign_lr("/=", ND_DIV);
    consume_assign_lr("&=", ND_BIT_AND);
    consume_assign_lr("|=", ND_BIT_OR);
    consume_assign_lr("^=", ND_BIT_XOR);
    consume_assign_lr("<<=", ND_BIT_SHIFT_L);
    consume_assign_lr(">>=", ND_BIT_SHIFT_R);
    return node;
}