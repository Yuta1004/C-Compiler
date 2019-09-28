#include "yncc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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

        // "+", "-" ...
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
           *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
           *p == ';' || *p == '=' || *p == '%' || *p == '{' ||
           *p == '}' || *p == ',' || *p == '&' || *p == '[' ||
           *p == ']'){
            cur = new_token(TOKEN_RESERVED, cur, p++);
            cur->len = 1;
            continue;
        }

        // 文字列
        if(*p == '\"'){
            // トークン生成
            ++ p;
            cur = new_token(TOKEN_STR, cur, p);

            // 文字列読み込み -> 登録
            int len = 0;
            do ++ len; while(*(p+len) != '\"');
            char *str = (char*)calloc(len, sizeof(char));
            strncpy(str, p, len);
            vec_push(str_vec, str);
            p += len + 1;
            cur->len = len;
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

        // while
        if(strncmp(p, "while", 5) == 0 && !is_alnum(*(p+5))){
            cur = new_token(TOKEN_WHILE, cur, p);
            cur->len = 5;
            p += 5;
            continue;
        }

        // for
        if(strncmp(p, "for", 3) == 0 && !is_alnum(*(p+3))){
            cur = new_token(TOKEN_FOR ,cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }

        // int
        if(strncmp(p, "int", 3) == 0 && !is_alnum(*(p+3))){
            cur = new_token(TOKEN_INT, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }

        // char
        if(strncmp(p, "char", 4) == 0 && !is_alnum(*(p+4))){
            cur = new_token(TOKEN_CHAR, cur, p);
            cur->len = 4;
            p += 4;
            continue;
        }

        // sizeof
        if(strncmp(p, "sizeof", 6) == 0 && !is_alnum(*(p+6))){
            cur = new_token(TOKEN_SIZEOF, cur, p);
            cur->len = 6;
            p += 6;
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
