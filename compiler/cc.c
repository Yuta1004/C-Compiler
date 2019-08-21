#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* 型定義 */
typedef enum {
    TOKEN_RESERVED,     // 記号
    TOKEN_NUM,          // 数字
    TOKEN_EOF,          // EOF
} TokenKind;

typedef struct Token Token;

typedef enum {
    ND_ADD,             // +
    ND_SUB,             // -
    ND_MUL,             // *
    ND_DIV,             // /
    ND_NUM,             // 数字
} NodeKind;

typedef struct Node Node;

/* 構造体 */
struct Token {
    TokenKind kind;     // トークンの種類
    Token *next;        // 次のトークンのポインタ
    int val;            // 値
    char *str;          // トークン文字列
    int len;            // トークン文字列の長さ
};

struct Node {
    NodeKind kind;      // ノードの種類
    Node *left;         // 左辺ノードのポインタ
    Node *right;        // 右辺ノードのポインタ
    int val;            // 数字ノードだった時、その値
};

/* グローバル変数 */
Token *token;
char *user_input;

/* プロトタイプ宣言 */
Node *expr();
Node *mul();
Node *term();
Node *unary();

/* 関数群 */
// エラー出力関数
void error(char *fmt, ...){
    va_list vargs;
    va_start(vargs, fmt);
    vfprintf(stderr, fmt, vargs);
    fprintf(stderr, "\n");
    exit(1);
}

//  エラー出力関数(詳細版)
void error_at(char *location, char *fmt, ...){
    va_list vargs;
    va_start(vargs, fmt);

    int err_pos = location - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", err_pos, "");
    fprintf(stderr, "^\t");
    vfprintf(stderr, fmt, vargs);
    fprintf(stderr, "\n\n");
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
        if(memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0 ||
           memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0){
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
// expr = mul ("+" mul | "-" mul)*
Node *expr(){
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

// 構文解析2
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

// 構文解析3
// term = num | "(" expr ")"
Node *term(){
    if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_num_node(expect_number());
}

// 構文解析4
// unary = ("+" | "-")? term
Node *unary(){
    if(consume("-")) {
        return new_node(ND_SUB, new_num_node(0), term());
    }

    return term();
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
    default:
        error("[ERROR] 構文木解析エラー");
    }

    printf("        push rax\n");
}

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "[ERROR] 引数が少なすぎます！\n");
        return 0;
    }

    // トークナイズ
    user_input = argv[1];
    token = tokenize(argv[1]);

    // 構文木生成
    Node *node_top = expr();

    // ヘッダー
    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");
    printf("main:\n");

    // アセンブリ出力
    gen_asm(node_top);

    // フッター
    printf("        pop rax\n");
    printf("        ret\n");
    return 0;
}
