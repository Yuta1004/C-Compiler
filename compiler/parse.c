#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"


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