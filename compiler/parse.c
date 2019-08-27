#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

/* プロトタイプ宣言 */
Node *program();
Node *stmt();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));     // Tokenサイズのメモリを1区間要求する(with ゼロクリア)
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 文字列をトークナイズして返す
Token *tokenize(char *p){
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p){
        // スペース
        if(isspace(*p)){
            ++ p;
            continue;
        }

        // ">=", "<=", "==", "!="
        if(strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
           strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0){
            cur = new_token(TOKEN_RESERVED, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }

        // "+", "-"
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
           *p == '(' || *p == ')' || *p == '>' || *p == '<'){
            cur = new_token(TOKEN_RESERVED, cur, p++);
            cur->len = 1;
            continue;
        }

        // 識別子(a~z)
        if('a' <= *p && *p <= 'z'){
            cur = new_token(TOKEN_IDENT, cur, p++);
            continue;
        }

        // 数字
        if(isdigit(*p)){
            cur = new_token(TOKEN_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズに失敗しました");
    }

    new_token(TOKEN_EOF, cur, NULL);
    return head.next;
}

// トークンが期待する文字かチェックする
// もし期待する文字なら1つトークンを進める
bool consume(char *op){
    if(token->kind == TOKEN_RESERVED && strlen(op) == token->len &&
            memcmp(token->str, op, token->len) == 0){
        token = token->next;
        return true;
    } else {
        return false;
    }
}

// 次のトークンを返す
// トークンの位置は1進む
Token *consume_ident(){
    Token *tmp = token;
    token = token->next;
    return tmp;
}

// トークンが期待する文字かチェックする
// もし期待する文字出なかった場合エラーを投げる
void expect(char *op){
    if(token->kind == TOKEN_RESERVED && strlen(op) == token->len &&
            memcmp(token->str, op, token->len) == 0){
        token = token->next;
    } else {
        error_at(token->str, "トークンが要求と異なります");
    }
}

// トークンが数字かチェックする
// 数字ならその数を、そうでなければエラーを投げる
int expect_number(){
    if(token->kind == TOKEN_NUM){
        int val = token->val;
        token = token->next;
        return val;
    } else {
        error_at(token->str, "トークンに数字が要求されました");
    }
}

// EOFチェック
bool at_eof(){
    return token->kind == TOKEN_EOF;
}

// ノード生成
Node *new_node(NodeKind kind, Node *left, Node *right){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

// 数字ノード生成
Node *new_num_node(int val){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 構文解析1
// program = stmt*
Node *program(){
    int idx = 0;
    for(; !at_eof(); ++ idx){
        code[idx] = stmt();
    }
    code[idx] = NULL;
}

// 構文解析2
// stmt = expr ";"
Node *stmt(){
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
// assign = equality ("=" assign)
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
        node->offset = (next_token->str[0] - 'a' + 1) * 8;
        return node;
    }

    return new_num_node(expect_number());
}
