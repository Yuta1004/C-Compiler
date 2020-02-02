#ifndef PARSE_H
#define PARSE_H

// スコープ管理
void in_scope();
void out_scope();


/* 上位 (優先度:低) */
Node *func();
Node *block();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *log_or();
Node *log_and();
Node *bit_or();
Node *bit_xor();
Node *bit_and();
Node *relational();
Node *bit_shift();
Node *add();
Node *mul();
Node *unary();
Node *accessor();
Node *primary();
/* 下位 (優先度:高) */

#endif 
