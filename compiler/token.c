#include "yncc.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


// トークンが期待する文字かチェックする
// もし期待する文字なら1つトークンを進める
bool consume(char *op){
    if(token->kind == TOKEN_RESERVED && strlen(op) == token->len &&
            memcmp(token->str, op, token->len) == 0){
        token = token->next;
        return true;
    } else {
        return false;
    }
}

// トークンが識別子かどかチェックする
// もしそうなら1つトークンを進める
Token *consume_ident(){
    if(token->kind == TOKEN_IDENT){
        Token *tmp = token;
        token = token->next;
        return tmp;
    } else {
        return NULL;
    }
}

// トークンが数字かチェックする
// もしそうなら1つトークンを進める
Token *consume_number(){
    if(token->kind == TOKEN_NUM){
        Token *tmp = token;
        token = token->next;
        return tmp;
    } else {
        return NULL;
    }
}

// トークンが期待する文字かチェックする
// もし期待する文字出なかった場合エラーを投げる
void expect(char *op){
    if(token->kind == TOKEN_RESERVED && strlen(op) == token->len &&
            memcmp(token->str, op, token->len) == 0){
        token = token->next;
    } else {
        error_at(token->str, "トークンが要求と異なります -> %s", op);
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
