#include "yncc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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
           *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
           *p == ';' || *p == '='){
            cur = new_token(TOKEN_RESERVED, cur, p++);
            cur->len = 1;
            continue;
        }

        // return
        if(strncmp(p, "return", 6) == 0 && !is_alnum(*(p+6))){
            cur = new_token(TOKEN_RETURN, cur, p);
            cur->len = 6;
            p += 6;
            continue;
        }

        // if
        if(strncmp(p, "if", 2) == 0 && !is_alnum(*(p+2))){
            cur = new_token(TOKEN_IF, cur, p);
            cur->len = 2;
            p += 2;
            continue;
        }

        // else
        if(strncmp(p, "else", 4) == 0 && !is_alnum(*(p+4))){
            cur = new_token(TOKEN_ELSE, cur, p);
            cur->len = 4;
            p += 4;
            continue;
        }

        // 識別子
        if(('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z')){
            cur = new_token(TOKEN_IDENT, cur, p);
            int len = 0;
            while(('a' <= *(p+len) && *(p+len) <= 'z') ||
                  ('A' <= *(p+len) && *(p+len) <= 'Z') ||
                  *(p+len) == '_'){
                ++ len;
            }
            cur->len = len;
            p += len;
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
