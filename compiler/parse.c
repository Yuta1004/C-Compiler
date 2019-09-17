#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

/* プロトタイプ宣言 */
void program();
Node *func();
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
LVar *regist_lvar(Token *request);

// 構文解析1
// program = func*
void program(){
    int idx = 0;
    for(; !at_eof(); ++ idx){
        code[idx] = func();
    }
    code[idx] = NULL;
}

// 構文解析2
// func = ident "(" (ident ","?)* ")" "{" stmt* "}"
Node *func(){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    free(locals);
    locals = calloc(1, sizeof(LVar));

    // 関数名
    Token *f_name_token = consume_ident();
    if(!f_name_token){
        error("[ERROR] 関数定義が要求されました");
    }
    node->f_name = calloc(f_name_token->len, sizeof(char));
    strncpy(node->f_name, f_name_token->str, f_name_token->len);

    // 引数リスト
    expect("(");
    node->args = calloc(6, sizeof(Node));
    for(int idx = 0; idx < 6; ++ idx) {
        Token *arg_token = consume_ident();
        if(arg_token && arg_token->str != NULL) {
            LVar *lvar = regist_lvar(arg_token);
            Node *arg_node = calloc(1, sizeof(Node));
            arg_node->kind = ND_LVER;
            arg_node->offset = lvar->offset;
            node->args[idx] = arg_node;
        }
        if(!consume(",")){
            break;
        }
    }
    expect(")");

    // 関数本体
    node->left = stmt();
    return node;
}

// 構文解析3
// stmt = expr ";"
//        | "{" stmt* "}"
//        | "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")"
Node *stmt(){
    if(token->kind == TOKEN_RETURN){
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->left = expr();
        expect(";");
        return node;
    }

    // Block
    if(consume("{")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        // stmt*
        Node *now_node = node;
        while(!consume("}")) {
            Node *new_node = stmt();
            now_node->block_next_node = new_node;
            now_node = new_node;
        }
        now_node->block_next_node = NULL;
        return node;
    }

    // if
    if(token->kind == TOKEN_IF) {
        // if ( expr ) stmt
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->left = expr();
        node->right = calloc(1, sizeof(Node));
        expect(")");
        node->right->left = stmt();

        // else stmt
        if(token->kind == TOKEN_ELSE) {
            token = token->next;
            node->right->right = stmt();
        }
        return node;
    }

    // while
    if(token->kind == TOKEN_WHILE) {
        // while ( expr ) stmt
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->left = expr();
        expect(")");
        node->right = stmt();
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

        // stmt
        node->right->left->right = stmt();
        return node;
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
}

// 構文解析8
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

// 構文解析9
// mul = unary ("*" unary | "-" unary)*
Node *mul(){
    Node *node = unary();

    while(true) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else if(consume("%")) {
            node = new_node(ND_DIV_REMAIN, node, unary());
        } else {
            return node;
        }
    }
}

// 構文解析10
// unary = ("+" | "-")? primary | ("*" | "&") unary
Node *unary(){
    if(consume("-")) {
        return new_node(ND_SUB, new_num_node(0), primary());
    }
    if(consume("*")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->left = unary();
        return node;
    }
    if(consume("&")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->left = unary();
        return node;
    }

    return primary();
}

// 構文解析11
// primary = num | ident ("(" (expr ","?)* ")")? | "(" expr ")"
Node *primary(){
    // "(" expr ")"
    if(consume("(")) {
        Node *node = expr();
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
            node->f_name = (char*)malloc(next_token->len * sizeof(char));
            strncpy(node->f_name, next_token->str, next_token->len);
            node->f_name[next_token->len] = '\0';

            // 引数
            node->args = calloc(6, sizeof(Node));
            for(int idx = 0; idx < 6; ++ idx) {
                Token *bef_token = token;
                if(consume_number() || consume_ident()){
                    token = bef_token;
                    node->args[idx] = expr();
                }
                if(!consume(",")){
                    break;
                }
            }
            expect(")");
            return node;
        }

        // 変数
        Node *node = calloc(1, sizeof(Node));
        LVar *result = find_lvar(next_token);
        node->offset = result->offset;
        node->kind = ND_LVER;
        return node;
    }

    return new_num_node(expect_number());
}

// ローカル変数検索
LVar *find_lvar(Token *request){
    // 検索
    for(LVar *var = locals; var; var = var->next){
        if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
            return var;
        }
    }

    char *name = (char*)malloc(request->len*sizeof(char));
    strncpy(name, request->str, request->len);
    error("[ERROR] 定義されていない変数です => %s\n", name);
}

// ローカル変数登録
LVar *regist_lvar(Token *request){
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = request->str;
    lvar->len = request->len;
    lvar->offset = locals->offset + 8;
    locals = lvar;
    return lvar;
}