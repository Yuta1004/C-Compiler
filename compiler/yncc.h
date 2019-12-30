#ifndef YNCC_H
#define YNCC_H

#include <stdlib.h>
#include "vector.h"

/* 型定義 */
typedef enum {
    TOKEN_NONE,
    TOKEN_RESERVED,     // 記号
    TOKEN_NUM,          // 数字
    TOKEN_IDENT,        // 識別子
    TOKEN_STR,          // 文字列リテラル
    TOKEN_RETURN,       // return
    TOKEN_IF,           // if
    TOKEN_ELSE,         // else
    TOKEN_WHILE,        // while
    TOKEN_FOR,          // for
    TOKEN_INT,          // int
    TOKEN_CHAR,         // char
    TOKEN_SIZEOF,       // sizeof
    TOKEN_BREAK,        // break
    TOKEN_CONTINUE,     // continue
    TOKEN_STRUCT,       // struct
    TOKEN_EOF,          // EOF
} TokenKind;

typedef struct Token Token;

typedef enum {
    ND_NONE,
    ND_EQ,              // ==
    ND_NEQ,             // !=
    ND_UPPERL,          // >
    ND_UPPEREQL,        // >=
    ND_ADD,             // +
    ND_SUB,             // -
    ND_MUL,             // *
    ND_DIV,             // /
    ND_DIV_REMAIN,      // %
    ND_PRE_INC,         // 前置インクリメント
    ND_PRE_DEC,         // 前置デクリメント
    ND_POST_INC,        // 後置インクリメント
    ND_POST_DEC,        // 後置デクリメント
    ND_ASSIGN,          // =
    ND_ADDR,            // &()
    ND_DEREF,           // *()
    ND_LVAR,            // ローカル変数
    ND_GVAR,            // グローバル変数
    ND_INIT_GVAR,       // グローバル変数初期化
    ND_INIT_ARRAY,      // 配列初期化式
    ND_FUNC,            // 関数
    ND_CALL_FUNC,       // 関数呼び出し
    ND_BLOCK,           // ブロック
    ND_RETURN,          // return
    ND_IF,              // if
    ND_WHILE,           // while
    ND_FOR,             // for
    ND_BREAK,           // break
    ND_CONTINUE,        // continue
    ND_NUM,             // 数字
    ND_STR,             // 文字列リテラル
} NodeKind;

typedef struct Node Node;
typedef struct Type Type;
typedef struct Var Var;

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

    Vector *node_list;          // ノードに属するノードのリスト

    int val;                    // 数字ノードだった時、その値

    int offset;                 // ローカル変数ノードだった時、そのオフセット
    Type *type;                 // 変数ノードだった時、その型

    char *name;                 // グローバル変数定義 or 関数ノードだった時、その名前
    Node **args;                // 関数ノードだった時、その引数
};

#define LOCAL 0
#define GLOBAL 1
struct Var {
    int var_type;    // LOCAL or GLOBAL
    Var *next;       // 次のVar
    char *name;      // 変数名
    int len;         // 長さ
    int offset;      // ローカル変数だった時、RBPからのオフセット
    Type *type;      // 型
    Node *init_expr; // グローバル変数だった時、初期化式のノード
    int scope_id;     // ネストのID
};

struct Type {
    enum{ NONE, INT, PTR, ARRAY, CHAR, STR } ty;
    struct Type *ptr_to;
    size_t bytesize;
    size_t alignment;
};

/* グローバル変数 */
Token *token;
char *program_body;
Node *code[100];
Vector *locals;
Vector *globals;
Vector *str_vec;
Vector *man_scope;
int label, label_if, label_loop, sum_offset, scope_id, scope_sum_id;

/* common.c */
void error(char *fmt, ...);
void error_at(char *location, char *fmt, ...);
void outtxt(char *fmt, ...);
void outasm(char *fmt, ...);
void outlabel(char *fmt, ...);
int is_alnum(char chr);

/* type.c */
int type_to_size(Type *type);
void define_type(Type **type_ptr, int type);
Type *max_type(Type *a, Type *b);
Type *read_type();
Type *get_base_type(Type *type);

/* tokenize.c */
#include <stdbool.h>
Token *tokenize();

/* token.c */
void expect(char *op);
Token *expect_ident();
int expect_number();
Token *expect_kind(TokenKind kind);
bool consume(char *op);
Token *consume_ident();
Token *consume_number();
Token *consume_kind(TokenKind kind);
bool at_eof();

/* node.c */
Node *new_node(NodeKind kind, Node *left, Node *right);
Node *new_num_node(int val);
Node *new_var_node(Var *var);
Node *new_none_node();

/* parse.c */
void program();

/* variable.c */
Var *find_var(Token *request);
Var *regist_var(int var_type);

/* string.c */
int find_str(char *target_str);

/* codegen.c */
void gen_asm();

/* precalc.c */
int precalc_expr(Node *root);
void decode_precalc_expr(char *s, Node *root);

/* struct.c */
bool regist_struct(char *tag);

#endif // YNCC_H