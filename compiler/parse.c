#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

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
Node *accessor();
Node *primary();

void in_scope() {
    scope_id = (++scope_sum_id);
    vec_push(man_scope, (void*)(long)scope_sum_id);
}

void out_scope() {
    vec_pop(man_scope);
    scope_id = (int)(long)vec_get(man_scope, man_scope->len-1);
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
    Node *node = new_node(ND_FUNC);
    vec_free(locals);
    locals = vec_new(10);
    sum_offset = 0;

    // 構造体(グローバル)
    if(consume_kind(TOKEN_STRUCT)) {
        Token *tag = expect_ident();
        if(def_struct(GLOBAL, tag->str, tag->len)) {
            expect(";");
            return new_none_node();
        }
        token = bef_token;
    }

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
                Node *arg_node = new_node(ND_LVAR);
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
    var->init_expr = new_none_node();
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

// 構文解析3
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

// 構文解析4
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

// 構文解析5
// assign = equality ("=" assign)?
Node *assign(){
    Node *node = equality();
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
    }
    return node;
}

// 構文解析6
// log_or = log_and ("&&" | log_and)

// 構文解析7
// log_and = bit_or ("||" | bit_or)

// 構文解析8
// bit_or = bit_xor ("|" bit_xor)?

// 構文解析9
// bit_xor = bit_and ("^" | bit_and)?

// 構文解析10
// bit_and = bit_and ("&" | bit_and)?

// 構文解析11
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

// 構文解析12
// relational = add (">" add | ">=" add | "<" add | "<=" add)*
Node *relational(){
    Node *node = add();
    if(consume(">")) {
        node = new_node_lr(ND_UPPERL, node, add());
    } else if(consume(">=")) {
        node = new_node_lr(ND_UPPEREQL, node, add());
    } else if(consume("<")) {
        node = new_node_lr(ND_UPPERL, add(), node);
    } else if(consume("<=")) {
        node = new_node_lr(ND_UPPEREQL, add(), node);
    }
    return node;
}

// 構文解析13
// add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();
    while(true) {
        if(consume("+")) {
            node = new_node_lr(ND_ADD, node, mul());
        } else if(consume("-")) {
            node = new_node_lr(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// 構文解析14
// mul = unary ("*" unary | "-" unary)*
Node *mul(){
    Node *node = unary();
    while(true) {
        if(consume("*")) {
            node = new_node_lr(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node_lr(ND_DIV, node, unary());
        } else if(consume("%")) {
            node = new_node_lr(ND_DIV_REMAIN, node, unary());
        } else {
            return node;
        }
    }
}

// 構文解析15
// unary = "sizeof" unary | ("+" | "-")? accessor | ("*" | "&") unary | unary "[" unary "]"  |
//         ("++" | "--") accessor
Node *unary(){
    if(consume_kind(TOKEN_SIZEOF)) {
        Node *node = unary();
        int bytesize = node->type->bytesize;
        free(node);
        return new_num_node(bytesize);
    }

    if(consume("-")) {
        return new_node_lr(ND_SUB, new_num_node(0), accessor());
    }

    if(consume("*")) {
        Node *node = new_node_lr(ND_DEREF, unary(), NULL);
        node->type = node->left->type->ptr_to;
        return node;
    }

    if(consume("&")) {
        Node *node = new_node_lr(ND_ADDR, unary(), NULL);
        define_type(&node->type, PTR);
        define_type(&node->type->ptr_to, PTR);
        return node;
    }

    if(consume("++")) { // 前置
        return new_node_lr(ND_PRE_INC, accessor(), NULL);
    }

    if(consume("--")) { // 前置
        return new_node_lr(ND_PRE_DEC, accessor(), NULL);
    }

    return accessor();
}

// 構文解析16
// accessor = primary ("++" | "--") | primary "[" (ident | num ) "]" | primary ("." | "->") ident
Node *accessor() {
    Node *node = primary();

    if(consume("++")) { // 後置
        return new_node_lr(ND_POST_INC, node, NULL);
    }

    if(consume("--")) { // 後置
        return new_node_lr(ND_POST_DEC, node, NULL);
    }

    while(true) {
        // "[" (ident | num ) "]"
        if(consume("[")) {
            Node *add = new_node_lr(ND_ADD, node, expr());
            node = new_node_lr(ND_DEREF, add, NULL);
            add->type = max_type(add->left->type->ptr_to, add->right->type->ptr_to);
            copy_type(&node->type, add->type);
            expect("]");
            continue;
        }

        // primary ("." | "->") ident
        bool arrowacs = false;
        if(consume(".") || (arrowacs=consume("->"))) {
            // メンバ名取得
            Var *member;
            Token *member_n = expect_ident();
            if(arrowacs) {
                node = new_node_lr(ND_DEREF, node, NULL);
                node->type = node->left->type->ptr_to;
            }
            member = struct_get_member(node->type->tag, node->type->len, member_n->str, member_n->len);
            if(!member)
                error_at(member_n->str, "構造体名またはメンバ名が正しくありません");

            // オフセット, 型設定
            Node *deref_node = new_node(ND_DEREF);
            deref_node->left = new_node(ND_ADD);
            deref_node->left->left = new_node_lr(ND_ADDR, node, NULL);
            deref_node->left->right = new_num_node(member->offset);
            copy_type(&deref_node->left->type, node->type);
            copy_type(&deref_node->left->type->ptr_to, member->type->ptr_to);
            copy_type(&deref_node->type, member->type);
            node = member->type->ty==ARRAY ? deref_node->left : deref_node;
            continue;
        }
        break;
    }
    return node;
}

// 構文解析17
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

// 構文解析18
// type = ("int" | "char") "*"*
//   -> type.c
