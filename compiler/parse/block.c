#include "../yncc.h"
#include "../vector/vector.h"
#include "parse.h"

// block = "{" block* "}" | stmt
Node *block() {
    // Block
    if(consume("{")) {
        Node *node = new_node(ND_BLOCK);
        node->node_list = vec_new(10);

        // stmt*
        in_scope();
        Node *now_node = node;
        while(!consume("}")) {
            vec_push(node->node_list, block());
        }
        out_scope();
        return node;
    }
    return stmt();
}
