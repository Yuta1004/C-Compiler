#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

/* プロトタイプ宣言 */
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
LVar *find_lvar(Token *request);

// 構文解析1
// program = stmt*
void program(){
    int idx = 0;
    for(; !at_eof(); ++ idx){
        code[idx] = stmt();
    }
    code[idx] = NULL;
}

// 構文解析2
// stmt = expr ";"
//        | "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
Node *stmt(){
    if(token->kind == TOKEN_RETURN){
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->left = expr();
        expect(";");
        return node;
    }

    if(token->kind == TOKEN_IF) {
        // if ( expr )
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        Node *condition = expr();
        expect(")");

        // stmt
        node->left = stmt();
        node->left->left = condition;

        // else stmt
        if(token->kind == TOKEN_ELSE) {
            token = token->next;
            node->right = stmt();
        }
        return node;
    }

    Node *node = expr();
    expect(";");
    return node;
}

// 構文解析3
// expr = assign
Node *expr(){
    return assign();
}

// 構文解析4
// assign = equality ("=" assign)?
Node *assign(){
    Node *node = equality();
    if(consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    } else {
        return node;
    }
}

// 構文解析5
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
}

// 構文解析6
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
}

// 構文解析7
// add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();

    while(true) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// 構文解析8
// mul = unary ("*" unary | "-" unary)*
Node *mul(){
    Node *node = unary();

    while(true) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

// 構文解析9
// unary = ("+" | "-")? term
Node *unary(){
    if(consume("-")) {
        return new_node(ND_SUB, new_num_node(0), primary());
    }

    return primary();
}

// 構文解析10
// primary = num | ident | "(" expr ")"
Node *primary(){
    if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *next_token = consume_ident();
    if(next_token){
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVER;

        LVar *result = find_lvar(next_token);       // 変数登録済みか確認
        if(result != NULL){
            node->offset = result->offset;
        } else {
            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = next_token->str;
            lvar->len = next_token->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }

    return new_num_node(expect_number());
}

// ローカル変数検索
LVar *find_lvar(Token *request){
    for(LVar *var = locals; var; var = var->next){
        if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
            return var;
        }
    }
    return NULL;
}