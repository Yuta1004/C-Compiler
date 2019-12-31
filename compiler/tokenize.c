#include "yncc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define def_token_stmt(word, w_len, kind) \
    if(strncmp(p, (word), (w_len)) == 0 && !is_alnum(*(p+(w_len)))){ \
        cur = new_token((kind), cur, p); \
        cur->len = (w_len); \
        p += (w_len); \
        continue; \
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

        // "//" 1行コメント
        if(strncmp(p, "//", 2) == 0) {
            while(*p != '\n') ++ p;
            ++p;
            continue;
        }

        // "/* */" 複数行コメント
        if(strncmp(p, "/*", 2) == 0) {
            while(strncmp(p, "*/", 2)) ++ p;
            p += 2;
            continue;
        }

        // ">=", "<=", "==", "!=" ...
        if(strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
           strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
           strncmp(p, "+=", 2) == 0 || strncmp(p, "-=", 2) == 0 ||
           strncmp(p, "*=", 2) == 0 || strncmp(p, "/=", 2) == 0 ||
           strncmp(p, "++", 2) == 0 || strncmp(p, "--", 2) == 0 ||
           strncmp(p, "->", 2) == 0) {
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
           *p == ']' || *p == '.'){
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
            char *str = (char*)calloc(len+1, sizeof(char));
            strncpy(str, p, len);
            vec_push(str_vec, str);
            p += len + 1;
            cur->len = len;
            continue;
        }

        // 予約語
        def_token_stmt("return", 6, TOKEN_RETURN);
        def_token_stmt("if", 2, TOKEN_IF);
        def_token_stmt("else", 4, TOKEN_ELSE);
        def_token_stmt("while", 5, TOKEN_WHILE);
        def_token_stmt("for", 3, TOKEN_FOR);
        def_token_stmt("int", 3, TOKEN_INT);
        def_token_stmt("char", 4, TOKEN_CHAR);
        def_token_stmt("sizeof", 6, TOKEN_SIZEOF);
        def_token_stmt("break", 5, TOKEN_BREAK);
        def_token_stmt("continue", 8, TOKEN_CONTINUE);
        def_token_stmt("struct", 6, TOKEN_STRUCT);

        // 識別子
        if(('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z')){
            cur = new_token(TOKEN_IDENT, cur, p);
            int len = 0;
            while(('a' <= *(p+len) && *(p+len) <= 'z') ||
                  ('A' <= *(p+len) && *(p+len) <= 'Z') ||
                  ('0' <= *(p+len) && *(p+len) <= '9') ||
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
