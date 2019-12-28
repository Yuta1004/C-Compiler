#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

#define set_lr_max_type(node) ((node)->type = max_type((node)->left->type, (node)->right->type))

/* プロトタイプ宣言 */
void in_scope();
void out_scope();
void program();
Node *func();
Node *block();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void in_scope() {
    scope_id = (++scope_sum_id);
    vec_push(man_scope, (void*)(long)scope_sum_id);
}

void out_scope() {
    scope_id = ((int)(long)vec_pop(man_scope)) - 1;
}

// 構文解析1
// program = func*
void program(){
    int idx = 0;
    for(; !at_eof(); ++ idx){
        code[idx] = func();
    }
    code[idx] = (Node*)-1;
}

// 構文解析2
// func = type ident "(" (type ident ","?)* ")" block
//      | type ident "*"* ("[" num "]")? ("=" equality)? ";"
Node *func(){
    Token *bef_token = token;

    // ローカル変数初期化
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    vec_free(locals);
    locals = vec_new(10);
    sum_offset = 0;

    // 関数名
    read_type();
    Token *f_name_token = consume_ident();
    if(!f_name_token){
        error_at(token->str, "関数定義が要求されました");
    }
    node->name = (char*)calloc(f_name_token->len+1, sizeof(char));
    strncpy(node->name, f_name_token->str, f_name_token->len);

    // 関数定義
    if(consume("(")) {
        in_scope();
        node->args = calloc(6, sizeof(Node));
        for(int idx = 0; idx < 6; ++ idx) {
            Var *var = regist_var(LOCAL);
            if(var) {                                      // 引数があるかチェック
                Node *arg_node = calloc(1, sizeof(Node));
                arg_node->kind = ND_LVAR;
                arg_node->offset = var->offset;
                node->args[idx] = arg_node;
            }
            if(!consume(",")){                              // , が無ければ続く引数は無いと判断する
                break;
            }
        }
        expect(")");
        node->left = block();
        out_scope();
        return node;
    }

    // グローバル変数定義
    token = bef_token;
    Var *var = regist_var(GLOBAL);
    var->init_expr = calloc(1, sizeof(Node));
    var->init_expr->kind = ND_NONE;
    var->init_expr->type = var->type;
    if(consume("=")) {
        free(var->init_expr);
        var->init_expr = expr();
    }
    expect(";");
    return new_none_node();
}

// 構文解析3
// block = "{" block* "}" | stmt
Node *block() {
    // Block
    if(consume("{")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
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

// 構文解析3
// stmt =   expr? ";"
//        | "return" expr ";"
//        | "if" "(" expr ")" block ("else" block)?
//        | "while" "(" expr ")" block
//        | "for" "(" expr? ";" expr? ";" expr? ")" block
//        | "break" ";"
//        | "continue" ";"
//        | type ident ("[" num "]")? ";"
Node *stmt(){
    if(token->kind == TOKEN_RETURN){
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->left = expr();
        expect(";");
        return node;
    }

    // if
    if(token->kind == TOKEN_IF) {
        in_scope();
        // if ( expr ) block
        ++ scope_id;
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->left = expr();
        node->right = calloc(1, sizeof(Node));
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
    if(token->kind == TOKEN_WHILE) {
        in_scope();
        // while ( expr ) block
        ++ scope_id;
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->left = expr();
        expect(")");
        node->right = block();
        out_scope();
        return node;
    }

    // for
    if(token->kind == TOKEN_FOR) {
        in_scope();
        // for (
        ++ scope_id;
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->right = calloc(1, sizeof(Node));
        node->right->left = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
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
            Node *node = new_node(ND_BLOCK, NULL, NULL);
            node->node_list = vec_new(init_expr->node_list->len+1);

            // 各要素を配列への代入式へ変換する
            for(int idx = 0; idx < init_expr->node_list->len; ++ idx) {
                Node *addr = new_node(ND_ADDR, var_node, NULL);                 // array
                Node *add_expr = new_node(ND_ADD, addr, new_num_node(idx));     // array+idx
                Node *left = new_node(ND_DEREF, add_expr, NULL);                // *(array+idx)
                Node *right = (Node*)vec_get(init_expr->node_list, idx);        // right
                define_type(&addr->type, PTR);
                define_type(&addr->type->ptr_to, addr->left->type->ptr_to->ty);
                add_expr->type = addr->type;
                left->type = add_expr->type->ptr_to;
                vec_push(node->node_list, new_node(ND_ASSIGN, left, right));    // *(array+idx) = right
            }
            return node;
        }
        return new_node(ND_ASSIGN, var_node, init_expr);
    }
    if(var){
        expect(";");
        return NULL;
    }

    // "break" ";"
    if(token->kind == TOKEN_BREAK) {
        token = token->next;
        expect(";");
        return new_node(ND_BREAK, NULL, NULL);
    }

    // "continue" ";"
    if(token->kind == TOKEN_CONTINUE) {
        token = token->next;
        expect(";");
        return new_node(ND_CONTINUE, NULL, NULL);
    }

    // ;
    if(consume(";")) {
        return NULL;
    }

    Node *node = expr();
    expect(";");
    return node;
}

// 構文解析4
// expr = assign | "{" expr ("," expr)* "}"
Node *expr(){
    // 配列の初期化式
    if(consume("{")) {
        // ノード生成
        Node *array_init_expr = calloc(1, sizeof(Node));
        array_init_expr->node_list = vec_new(10);

        // 初期化式
        int size;
        for(size = 0; !consume("}"); ++ size) {
            if(size > 0)
                expect(",");
            vec_push(array_init_expr->node_list, expr());
        }
        array_init_expr->val = size;
        array_init_expr->kind = ND_INIT_ARRAY;
        return array_init_expr;
    }

    // 式
    return assign();
}

// 構文解析5
// assign = equality ("=" assign)?
Node *assign(){
    bool is_comp_assign = true;
    Node *node = equality();

    if(consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
        is_comp_assign = false;
    } else if(consume("+=")){
        node = new_node(ND_ASSIGN, node, new_node(ND_ADD, node, assign()));
    } else if(consume("-=")){
        node = new_node(ND_ASSIGN, node, new_node(ND_SUB, node, assign()));
    } else if(consume("*=")){
        node = new_node(ND_ASSIGN, node, new_node(ND_MUL, node, assign()));
    } else if(consume("/=")){
        node = new_node(ND_ASSIGN, node, new_node(ND_DIV, node, assign()));
    } else {
        return node;
    }

    set_lr_max_type(node);
    if(is_comp_assign)
        set_lr_max_type(node->right);
    return node;
}

// 構文解析6
// equality = relational ("==" relational | "!=" relational)*
Node *equality(){
    Node *node = relational();

    if(consume("==")) {
        node = new_node(ND_EQ, node, relational());
    } else if(consume("!=")) {
        node = new_node(ND_NEQ, node, relational());
    } else {
        return node;
    }
    set_lr_max_type(node);
    return node;
}

// 構文解析7
// relational = add (">" add | ">=" add | "<" add | "<=" add)*
Node *relational(){
    Node *node = add();

    // <, <= は両辺入れ替えて >, >= と同じように扱う(発想の勝利)
    if(consume(">")) {
        node = new_node(ND_UPPERL, node, add());
    } else if(consume(">=")) {
        node = new_node(ND_UPPEREQL, node, add());
    } else if(consume("<")) {
        node = new_node(ND_UPPERL, add(), node);
    } else if(consume("<=")) {
        node = new_node(ND_UPPEREQL, add(), node);
    } else {
        return node;
    }
    set_lr_max_type(node);
    return node;
}

// 構文解析8
// add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();

    while(true) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
            set_lr_max_type(node);
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
            set_lr_max_type(node);
        } else {
            return node;
        }
    }
}

// 構文解析9
// mul = unary ("*" unary | "-" unary)*
Node *mul(){
    Node *node = unary();

    while(true) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
            set_lr_max_type(node);
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
            set_lr_max_type(node);
        } else if(consume("%")) {
            node = new_node(ND_DIV_REMAIN, node, unary());
            set_lr_max_type(node);
        } else {
            return node;
        }
    }
}

// 構文解析10
// unary = "sizeof" unary | ("+" | "-")? primary | ("*" | "&") unary | unary "[" unary "]"  |
//         ("++" | "--") primary
Node *unary(){
    if(token->kind == TOKEN_SIZEOF) {
        token = token->next;
        Node *node = unary();
        int type = node->type->ty;
        free(node);
        return new_num_node(type_to_size(node->type));
    }

    if(consume("-")) {
        Node *node = new_node(ND_SUB, new_num_node(0), primary());
        set_lr_max_type(node);
        return node;
    }

    Node *node = NULL;
    if(consume("*")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->left = unary();
        node->type = node->left->type->ptr_to;
        goto check_array_access;
    }

    if(consume("&")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->left = unary();
        define_type(&node->type, PTR);
        define_type(&node->type->ptr_to, PTR);
        goto check_array_access;
    }

    if(consume("++")) { // 前置
        node = calloc(1, sizeof(Node));
        node->kind = ND_PRE_INC;
        node->left = primary();
        define_type(&node->type, node->left->type->ty);
        goto check_array_access;
    }

    if(consume("--")) { // 前置
        node = calloc(1, sizeof(Node));
        node->kind = ND_PRE_DEC;
        node->left = primary();
        define_type(&node->type, node->left->type->ty);
        goto check_array_access;
    }

    if(node == NULL) {
        node = primary();
        goto check_array_access;
    }

    // "[" (ident | num ) "]"
check_array_access:
    if(consume("[")) {
        Node *deref_par = calloc(1, sizeof(Node));
        Node *add = calloc(1, sizeof(Node));
        deref_par->kind = ND_DEREF;
        deref_par->left = add;
        add->kind = ND_ADD;
        add->left = node;
        add->right = expr();
        add->type = max_type(add->left->type->ptr_to, add->right->type->ptr_to);
        define_type(&deref_par->type, add->type->ty);
        expect("]");
        return deref_par;
    }

    if(consume("++")) { // 後置
        Node *tmp = calloc(1, sizeof(Node));
        tmp->kind = ND_POST_INC;
        tmp->left = node;
        define_type(&tmp->type, node->type->ty);
        node = tmp;
    }

    if(consume("--")) { // 後置
        Node *tmp = calloc(1, sizeof(Node));
        tmp->kind = ND_POST_DEC;
        tmp->left = node;
        define_type(&tmp->type, node->type->ty);
        node = tmp;
    }

    return node;
}

// 構文解析11
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
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_CALL_FUNC;
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
        Node *node = calloc(1, sizeof(Node));
        Var *result = find_var(next_token);
        node->kind = ND_LVAR;
        node->offset = result->offset;
        node->type = result->type;
        node->name = result->name;
        if(result->var_type == GLOBAL) node->kind = ND_GVAR;

        // 変数が配列を指していた場合、先頭アドレスへのポインタに変換する
        if(node->type->ty == ARRAY) {
            node = new_node(ND_ADDR, node, NULL);
            define_type(&node->type, PTR);
            define_type(&node->type->ptr_to, node->left->type->ptr_to->ty);
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

        Node *node = (Node*)calloc(1, sizeof(Node));
        node->kind = ND_STR;
        node->val = str_id;
        define_type(&node->type, STR);
        return node;
    }

    return new_num_node(expect_number());
}

// 構文解析12
// type = ("int" | "char") "*"*
//   -> type.c
