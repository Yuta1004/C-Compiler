#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yncc.h"

/* プロトタイプ宣言 */
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

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
// expr = equality
Node *expr(){
    return equality();
}

// 構文解析2
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

// 構文解析3
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

// 構文解析4
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

// 構文解析5
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

// 構文解析6
// unary = ("+" | "-")? term
Node *unary(){
    if(consume("-")) {
        return new_node(ND_SUB, new_num_node(0), primary());
    }

    return primary();
}

// 構文解析7
// primary = num | "(" expr ")"
Node *primary(){
    if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_num_node(expect_number());
}

// 構文木 to アセンブリ
void gen_asm(Node *node){
    if(node->kind == ND_NUM) {
        printf("        push %d\n", node->val);
        return;
    }

    gen_asm(node->left);
    gen_asm(node->right);

    printf("        pop rdi\n");
    printf("        pop rax\n");

    switch(node->kind){
    case ND_ADD:
        printf("        add rax, rdi\n");
        break;
    case ND_SUB:
        printf("        sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("        imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("        cqo\n");        // raxレジスタをrdxと合わせた128bitに拡張
        printf("        idiv rdi\n");   // rax / rsiの結果 (余りはrdx)
        break;
    case ND_EQ:
        printf("        cmp rdi, rax\n");   // rdiとraxを比較 -> 結果はフラグレジスタへ
        printf("        sete al\n");        // 比較結果(==)をalに入れる(raxの下位8ビットにあたるレジスタ)
        printf("        movzb rax, al\n");  // raxレジスタの上位56ビットをゼロクリア
        break;
    case ND_NEQ:
        printf("        cmp rdi, rax\n");
        printf("        setne al\n");       // 比較結果(!=)をalに入れる
        printf("        movzb rax, al\n");
        break;
    case ND_UPPERL:
        printf("        cmp rdi, rax\n");
        printf("        setl al\n");        // 比較結果(>)をalに入れる
        printf("        movzb rax, al\n");
        break;
    case ND_UPPEREQL:
        printf("        cmp rdi, rax\n");
        printf("        setle al\n");       // 比較結果(>=)をalに入れる
        printf("        movzb rax, al\n");
        break;
    default:
        error("[ERROR] 構文木解析エラー");
    }

    printf("        push rax\n");
}