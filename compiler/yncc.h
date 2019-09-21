#ifndef YNCC_H
#define YNCC_H

#include <stdlib.h>

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
    TOKEN_SIZEOF,       // sizeof
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
    ND_DEFGVAR,         // グローバル変数定義
    ND_VAR,             // 変数
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
typedef struct Type Type;
typedef struct LVar LVar;
typedef struct GVar GVar;

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
    Type *type;                 // 変数ノードだった時、その型

    char *name;                 // グローバル変数定義 or 関数ノードだった時、その名前
    Node **args;                // 関数ノードだった時、その引数
};

struct LVar {
    LVar *next;     // 次のLVar
    char *name;     // 変数名
    int len;        // 長さ
    int offset;     // RBPからのオフセット
    Type *type;     // 型
};

struct GVar {
    GVar *next;     // 次のGVar
    char *name;     // 変数名
    int len;        // 長さ
    Type *type;     // 型
};

struct Type {
    enum{ INT, PTR, ARRAY } ty;
    struct Type *ptr_to;
    size_t size;
};

/* グローバル変数 */
Token *token;
char *user_input;
Node *code[100];
LVar *locals;
GVar *globals;
int label;

/* common.c */
void error(char *fmt, ...);
void error_at(char *location, char *fmt, ...);
int is_alnum(char chr);
int type_to_size(Type *type);
void define_type(Type **type_ptr, int type);
Type *max_type(Type *a, Type *b);

/* tokenize.c */
#include <stdbool.h>
Token *tokenize();

/* token.c */
bool at_eof();
int expect_number();
bool consume(char *op);
void expect(char *op);
bool consume(char *op);
Token *expect_kind(TokenKind kind);
Token *consume_ident();
Token *consume_number();
Token *consume_kind();

/* node.c */
Node *new_node(NodeKind kind, Node *left, Node *right);
Node *new_num_node(int val);
Node *new_def_gvar_node(GVar *gvar);

/* parse.c */
void program();

/* variable.c */
LVar *find_lvar(Token *request);
void *regist_var(bool is_global);

/* codegen.c */
void gen_asm();

#endif // YNCC_H