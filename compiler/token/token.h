#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

/* 列挙体 */
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
    TOKEN_DO,           // do
    TOKEN_TYPEDEF,      // typedef
    TOKEN_EOF,          // EOF
} TokenKind;


/* 構造体 */
typedef struct Token Token;
struct Token {
    TokenKind kind;     // トークンの種類
    Token *next;        // 次のトークンのポインタ
    int val;            // 値
    char *str;          // トークン文字列
    int len;            // トークン文字列の長さ
};

/* tokenize.c */
Token *tokenize();

/* utils.c */
void expect(char *op);
int expect_number();
Token *expect_ident();
Token *expect_kind(TokenKind kind);
bool consume(char *op);
Token *consume_ident();
Token *consume_number();
Token *consume_kind(TokenKind kind);
bool at_eof();

#endif