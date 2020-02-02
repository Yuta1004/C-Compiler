#include <stdlib.h>

#include "../yncc.h"
#include "../vector.h"
#include "parse.h"

// stmt =   expr? ";"
//        | "return" expr ";"
//        | "if" "(" expr ")" block ("else" block)?
//        | "while" "(" expr ")" block
//        | "do" block "while" "(" expr ")" ";"
//        | "for" "(" expr? ";" expr? ";" expr? ")" block
//        | "break" ";"
//        | "continue" ";"
//        | type ident ("[" num "]")? ";"
//        | "struct" ident "{" type ident ";" )* "}" ";"
Node *stmt(){
    if(consume_kind(TOKEN_RETURN)){
        Node *node = new_node_lr(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    }

    // if
    if(consume_kind(TOKEN_IF)) {
        in_scope();
        // if ( expr ) block
        expect("(");
        Node *node = new_node_lr(ND_IF, expr(), new_none_node());
        expect(")");
        node->right->left = block();

        // else block
        if(token->kind == TOKEN_ELSE) {
            token = token->next;
            node->right->right = block();
        }
        out_scope();
        return node;
    }

    // while
    if(consume_kind(TOKEN_WHILE)) {
        in_scope();
        // while ( expr ) block
        expect("(");
        Node *node = new_node_lr(ND_WHILE, expr(), new_none_node());
        expect(")");
        node->right = block();
        out_scope();
        return node;
    }

    // do-while
    if(consume_kind(TOKEN_DO)) {
        in_scope();
        // "do" block
        Node *node = new_node_lr(ND_DO_WHILE, block(), NULL);
        // "while" "(" expr ")" ";"
        expect_kind(TOKEN_WHILE);
        expect("(");
        node->right = expr();
        expect(")");
        expect(";");
        return node;
    }

    // for
    if(consume_kind(TOKEN_FOR)) {
        in_scope();
        // for (
        Node *node = new_node_lr(ND_FOR, NULL, new_none_node());
        node->right->left = calloc(1, sizeof(Node));
        expect("(");

        // expr? ; <Init>
        if(!consume(";")) {
            if(token->kind == TOKEN_INT || token->kind == TOKEN_CHAR)
                node->left = stmt();
            else
                node->left = expr();
            expect(";");
        }
        // expr? ; <Condition>
        if(!consume(";")) {
            node->right->left->left = expr();
            expect(";");
        } else {
            node->right->left->left = new_num_node(1);
        }
        // expr? ; <Next>
        if(!consume(")")) {
            node->right->right = expr();
            expect(")");
        }

        // block
        node->right->left->right = block();
        out_scope();
        return node;
    }

    // ローカル変数定義
    Var *var = regist_var(LOCAL);
    if(var && consume("=")) {
        Node *var_node = new_var_node(var);
        Node *init_expr = expr();

        // 配列初期化式 or 普通の式
        if(init_expr->kind == ND_INIT_ARRAY) {
            // Blockノード初期化
            Node *node = new_node_lr(ND_BLOCK, NULL, NULL);
            node->node_list = vec_new(init_expr->node_list->len+1);

            // 各要素を配列への代入式へ変換する
            for(int idx = 0; idx < init_expr->node_list->len; ++ idx) {
                Node *addr = new_node_lr(ND_ADDR, var_node, NULL);                 // array
                Node *add_expr = new_node_lr(ND_ADD, addr, new_num_node(idx));     // array+idx
                Node *left = new_node_lr(ND_DEREF, add_expr, NULL);                // *(array+idx)
                Node *right = (Node*)vec_get(init_expr->node_list, idx);        // right
                define_type(&addr->type, PTR);
                copy_type(&addr->type->ptr_to, addr->left->type->ptr_to);
                add_expr->type = addr->type;
                left->type = add_expr->type->ptr_to;
                vec_push(node->node_list, new_node_lr(ND_ASSIGN, left, right));    // *(array+idx) = right
            }
            return node;
        }
        return new_node_lr(ND_ASSIGN, var_node, init_expr);
    }
    if(var){
        expect(";");
        return NULL;
    }

    // "break" ";"
    if(consume_kind(TOKEN_BREAK)) {
        expect(";");
        return new_node_lr(ND_BREAK, NULL, NULL);
    }

    // "continue" ";"
    if(consume_kind(TOKEN_CONTINUE)) {
        expect(";");
        return new_node_lr(ND_CONTINUE, NULL, NULL);
    }

    // "struct" ident "{" ... "}"
    if(consume_kind(TOKEN_STRUCT)) {
        Token *tag = expect_ident();
        def_struct(LOCAL, tag->str, tag->len);
        expect(";");
        return NULL;
    }

    // ;
    if(consume(";")) {
        return NULL;
    }

    Node *node = expr();
    expect(";");
    return node;
}