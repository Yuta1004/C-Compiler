#ifndef YNCC_H
#define YNCC_H

/* 型定義 */
typedef enum {
    TOKEN_RESERVED,     // 記号
    TOKEN_NUM,          // 数字
    TOKEN_IDENT,        // 識別子
    TOKEN_EOF,          // EOF
} TokenKind;

typedef struct Token Token;

typedef enum {
    ND_EQ,              // ==
    ND_NEQ,             // !=
    ND_UPPERL,          // >
    ND_UPPEREQL,        // >=
    ND_ADD,             // +
    ND_SUB,             // -
    ND_MUL,             // *
    ND_DIV,             // /
    ND_ASSIGN,          // =
    ND_LVER,           // ローカル変数
    ND_NUM,             // 数字
} NodeKind;

typedef struct Node Node;

typedef struct LVar LVar;

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
    int offset;         // ローカル変数ノードだった時、そのオフセット
};

struct LVar {
    LVar *next;     // 次のLVar
    char *name;     // 変数名
    int len;        // 長さ
    int offset;     // RBPからのオフセット
};

/* グローバル変数 */
Token *token;
char *user_input;
Node *code[100];
LVar *locals;

/* common.c */
void error(char *fmt, ...);
void error_at(char *location, char *fmt, ...);

/* parce.c */
Token *tokenize();
void program();

/* codegen.c */
void gen_asm();

#endif // YNCC_H