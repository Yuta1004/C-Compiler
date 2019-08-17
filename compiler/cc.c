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

/* 構造体 */
struct Token {
    TokenKind kind;     // トークンの種類
    Token *next;        // 次のトークンのポインタ
    int val;            // 値
    char *str;          // トークン文字列
};

/* グローバル変数 */
Token *token;
char *user_input;

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
bool consume(char op){
    if(token->kind == TOKEN_RESERVED && token->str[0] == op){
        token = token->next;
        return true;
    } else {
        return false;
    }
}

// トークンが期待する文字かチェックする
// もし期待する文字出なかった場合エラーを投げる
void expect(char op){
    if(token->kind == TOKEN_RESERVED && token->str[0] == op){
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

        // "+", "-"
        if(*p == '+' || *p == '-'){
            cur = new_token(TOKEN_RESERVED, cur, p++);
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


int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "[ERROR] 引数が少なすぎます！\n");
        return 0;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);

    printf(".intel_syntax   noprefix\n");
    printf(".global         main\n");
    printf("\n");
    printf("main:\n");
    printf("        mov rax, %d\n", expect_number());

    while(!at_eof()){
        // 足し算, 引き算
        if(consume('+')){
            printf("        add rax, %d\n", expect_number());
            continue;
        }
        if(consume('-')){
            printf("        sub rax, %d\n", expect_number());
            continue;
        }

        error_at(token->str, "構文エラー");
        exit(1);
    }

    printf("        ret\n");
    return 0;
}
