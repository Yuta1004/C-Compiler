#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

/* プロトタイプ宣言 */
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
//      | type ident "*"* ("[" expr "]")? ";"
Node *func(){
    Token *bef_token = token;

    // ローカル変数初期化
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    free(locals);
    locals = calloc(1, sizeof(Var));

    // 関数名
    read_type();
    Token *f_name_token = consume_ident();
    if(!f_name_token){
        error("[ERROR] 関数定義が要求されました");
    }
    node->name = (char*)calloc(f_name_token->len+1, sizeof(char));
    strncpy(node->name, f_name_token->str, f_name_token->len);

    // 関数定義 or グローバル変数定義分岐
    if(consume("(")) {          // 関数定義(引数リスト)
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
        return node;
    } else {                    // グローバル変数定義
        token = bef_token;
        Var *var = regist_var(GLOBAL);
        expect(";");
        return new_none_node();
    }
}

// 構文解析3
// block = "{" block* "}" | stmt
Node *block() {
    // Block
    if(consume("{")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        // stmt*
        Node *now_node = node;
        while(!consume("}")) {
            Node *new_node = block();
            if(new_node){
                now_node->block_next_node = new_node;
                now_node = new_node;
            }
        }
        now_node->block_next_node = NULL;
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
        // if ( expr ) block
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
        return node;
    }

    // while
    if(token->kind == TOKEN_WHILE) {
        // while ( expr ) block
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->left = expr();
        expect(")");
        node->right = block();
        return node;
    }

    // for
    if(token->kind == TOKEN_FOR) {
        // for (
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->right = calloc(1, sizeof(Node));
        node->right->left = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");

        // expr? ; <Init>
        if(!consume(";")) {
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
        return node;
    }

    // Variable
    if(regist_var(LOCAL)) {
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
// expr = assign
Node *expr(){
    return assign();
}

// 構文解析5
// assign = equality ("=" assign)?
Node *assign(){
    Node *node = equality();
    if(consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    } else {
        return node;
    }
    node->type = max_type(node->left->type, node->right->type);
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

    node->type = max_type(node->left->type, node->right->type);
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

    node->type = max_type(node->left->type, node->right->type);
    return node;
}

// 構文解析8
// add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();

    while(true) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
            node->type = max_type(node->left->type, node->right->type);
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
            node->type = max_type(node->left->type, node->right->type);
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
            node->type = max_type(node->left->type, node->right->type);
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
            node->type = max_type(node->left->type, node->right->type);
        } else if(consume("%")) {
            node = new_node(ND_DIV_REMAIN, node, unary());
            node->type = max_type(node->left->type, node->right->type);
        } else {
            return node;
        }
    }
}

// 構文解析10
// unary = "sizeof" unary | ("+" | "-")? primary | ("*" | "&") unary | unary "[" unary "]"
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
        node->type = max_type(node->left->type, node->right->type);
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
        node->offset = result->offset;
        node->type = result->type;
        node->kind = ND_LVAR;
        node->name = result->name;
        if(result->var_type == GLOBAL) node->kind = ND_GVAR;

        // 変数が配列を指していた場合、先頭アドレスへのポインタに変換する
        if(node->type->ty == ARRAY) {
            free(node);
            Node *addr_par = calloc(1, sizeof(Node));
            Node *addr = calloc(1, sizeof(Node));
            addr_par->kind = ND_ADDR;
            addr_par->left = addr;
            addr->kind = ND_LVAR;
            addr->offset = result->offset;
            addr->name = result->name;
            if(result->var_type == GLOBAL) addr->kind = ND_GVAR;
            define_type(&addr_par->type, PTR);
            define_type(&addr_par->type->ptr_to, result->type->ptr_to->ty);
            node = addr_par;
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