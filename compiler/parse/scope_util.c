#include "../yncc.h"
#include "parse.h"

void in_scope() {
    scope_id = (++scope_sum_id);
    vec_push(man_scope, (void*)(long)scope_sum_id);
}

void out_scope() {
    vec_pop(man_scope);
    scope_id = (int)(long)vec_get(man_scope, man_scope->len-1);
}
