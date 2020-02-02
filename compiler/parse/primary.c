#include <stdlib.h>
#include <string.h>

#include "../yncc.h"
#include "parse.h"

// primary = "(" (expr | block) ") | ident ("(" (expr ","?)* ")") | num | str | ident
Node *primary(){
    // "(" expr | block ")"
    if(consume("(")) {
        Node *node;
        Token *bef_token = token;
        if(consume("{")) {          // block
            token = bef_token;
            node = block();
        } else {                    // expr
            node = expr();
        }
        expect(")");
        return node;
    }

    Token *next_token = consume_ident();
    if(next_token){
        // 関数呼び出し
        if(consume("(")) {
            // 関数名
            Node *node = new_node(ND_CALL_FUNC);
            node->name = (char*)calloc(next_token->len+1, sizeof(char));
            define_type(&node->type, INT);
            strncpy(node->name, next_token->str, next_token->len);

            // 引数
            node->args = calloc(6, sizeof(Node));
            for(int idx = 0; idx < 6; ++ idx) {
                if(strncmp(token->str, ")", 1) == 0) break;
                node->args[idx] = expr();
                if(!consume(",")) break;
            }
            expect(")");
            return node;
        }

        // 変数
        Node *node = new_node(ND_LVAR);
        Var *result = find_var(next_token);
        node->offset = result->offset;
        node->type = result->type;
        node->name = result->name;
        if(result->var_type == GLOBAL) node->kind = ND_GVAR;

        // 変数が配列を指していた場合、先頭アドレスへのポインタに変換する
        if(node->type->ty == ARRAY) {
            int bytesize = node->type->bytesize;
            node = new_node_lr(ND_ADDR, node, NULL);
            define_type(&node->type, PTR);
            copy_type(&node->type->ptr_to, node->left->type->ptr_to);
            node->type->bytesize = bytesize;
        }
        return node;
    }

    // 文字列リテラル
    Token *bef_token = token;
    if(consume_kind(TOKEN_STR)) {
        // 文字列ID取得
        token = bef_token;
        int str_id = find_str(token->str);
        token = token->next;

        Node *node = new_node(ND_STR);
        node->val = str_id;
        define_type(&node->type, STR);
        return node;
    }

    return new_num_node(expect_number());
}
