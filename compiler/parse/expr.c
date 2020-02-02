#include "../yncc.h"
#include "../vector/vector.h"
#include "parse.h"

// expr = assign | "{" expr ("," expr)* "}"
Node *expr(){
    // 配列の初期化式
    if(consume("{")) {
        // ノード生成
        Node *array_init_expr = new_node(ND_INIT_ARRAY);
        array_init_expr->node_list = vec_new(10);

        // 初期化式
        int size;
        for(size = 0; !consume("}"); ++ size) {
            if(size > 0)
                expect(",");
            vec_push(array_init_expr->node_list, expr());
        }
        array_init_expr->val = size;
        return array_init_expr;
    }

    // 式
    return assign();
}