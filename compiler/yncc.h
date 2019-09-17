#ifndef YNCC_H
#define YNCC_H

/* 型定義 */
typedef enum {
    TOKEN_RESERVED,     // 記号
    TOKEN_NUM,          // 数字
    TOKEN_IDENT,        // 識別子
    TOKEN_RETURN,       // return
    TOKEN_IF,           // if
    TOKEN_ELSE,         // else
    TOKEN_WHILE,        // while
    TOKEN_FOR,          // for
    TOKEN_INT,          // int
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
    ND_DIV_REMAIN,      // %
    ND_ASSIGN,          // =
    ND_ADDR,            // &()
    ND_DEREF,           // *()
    ND_LVER,            // ローカル変数
    ND_FUNC,            // 関数
    ND_CALL_FUNC,       // 関数呼び出し
    ND_BLOCK,           // ブロック
    ND_RETURN,          // return
    ND_IF,              // if
    ND_WHILE,           // while
    ND_FOR,             // for
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
    NodeKind kind;              // ノードの種類
    Node *left;                 // 左辺ノードのポインタ
    Node *right;                // 右辺ノードのポインタ
    Node *block_next_node;      // ブロックノードだった時、その次のノード
    int val;                    // 数字ノードだった時、その値
    int offset;                 // ローカル変数ノードだった時、そのオフセット
    char *f_name;               // 関数ノードだった時、その名前
    Node **args;                  // 関数ノードだった時、その引数
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
int label;

/* common.c */
void error(char *fmt, ...);
void error_at(char *location, char *fmt, ...);
int is_alnum(char chr);

/* tokenize.c */
#include <stdbool.h>
Token *tokenize();

/* token.c */
bool at_eof();
int expect_number();
bool consume(char *op);
void expect(char *op);
bool consume(char *op);
Token *consume_ident();
Token *consume_number();

/* node.c */
Node *new_node(NodeKind kind, Node *left, Node *right);
Node *new_num_node(int val);

/* parse.c */
void program();

/* codegen.c */
void gen_asm();

#endif // YNCC_H