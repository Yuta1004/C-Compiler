#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "yncc.h"

#define LOCAL 0
#define GLOBAL 1

/* プロトタイプ宣言 */
void program();
Node *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
LVar *find_lvar(Token *request);
LVar *regist_var(bool is_global);

// 構文解析1
// program = func*
void program(){
    int idx = 0;
    for(; !at_eof(); ++ idx){
        code[idx] = func();
    }
    code[idx] = NULL;
}

// 構文解析2
// func = "int" ident "(" ("int" ident ","?)* ")" "{" stmt* "}"
Node *func(){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    free(locals);
    locals = calloc(1, sizeof(LVar));

    // 関数名
    expect_kind(TOKEN_INT);
    Token *f_name_token = consume_ident();
    if(!f_name_token){
        error("[ERROR] 関数定義が要求されました");
    }
    node->f_name = calloc(f_name_token->len, sizeof(char));
    strncpy(node->f_name, f_name_token->str, f_name_token->len);

    // 引数リスト
    expect("(");
    node->args = calloc(6, sizeof(Node));
    for(int idx = 0; idx < 6; ++ idx) {
        LVar *lvar = regist_var(LOCAL);
        if(lvar) {                                      // 引数があるかチェック
            Node *arg_node = calloc(1, sizeof(Node));
            arg_node->kind = ND_LVER;
            arg_node->offset = lvar->offset;
            node->args[idx] = arg_node;
        }
        if(!consume(",")){                              // , が無ければ続く引数は無いと判断する
            break;
        }
    }
    expect(")");

    // 関数本体
    node->left = stmt();
    return node;
}

// 構文解析3
// stmt = expr ";"
//        | "{" stmt* "}"
//        | "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")"
//        | "int" "*"* ident ("[" num "]")? ";"
Node *stmt(){
    if(token->kind == TOKEN_RETURN){
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->left = expr();
        expect(";");
        return node;
    }

    // Block
    if(consume("{")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;

        // stmt*
        Node *now_node = node;
        while(!consume("}")) {
            Node *new_node = stmt();
            if(new_node){
                now_node->block_next_node = new_node;
                now_node = new_node;
            }
        }
        now_node->block_next_node = NULL;
        return node;
    }

    // if
    if(token->kind == TOKEN_IF) {
        // if ( expr ) stmt
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->left = expr();
        node->right = calloc(1, sizeof(Node));
        expect(")");
        node->right->left = stmt();

        // else stmt
        if(token->kind == TOKEN_ELSE) {
            token = token->next;
            node->right->right = stmt();
        }
        return node;
    }

    // while
    if(token->kind == TOKEN_WHILE) {
        // while ( expr ) stmt
        token = token->next;
        expect("(");
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->left = expr();
        expect(")");
        node->right = stmt();
        return node;
    }

    // for
    if(token->kind == TOKEN_FOR) {
        // for (
        token = token->next;
        Node *node = calloc(1, sizeof(Node));
        node->right = calloc(1, sizeof(Node));
        node->right->left = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");

        // expr? ; <Init>
        if(!consume(";")) {
            node->left = expr();
            expect(";");
        }
        // expr? ; <Condition>
        if(!consume(";")) {
            node->right->left->left = expr();
            expect(";");
        } else {
            node->right->left->left = new_num_node(1);
        }
        // expr? ; <Next>
        if(!consume(")")) {
            node->right->right = expr();
            expect(")");
        }

        // stmt
        node->right->left->right = stmt();
        return node;
    }

    // Variable<int>
    if(token->kind == TOKEN_INT) {
        regist_var(LOCAL);
        expect(";");
        return NULL;
    }

    Node *node = expr();
    expect(";");
    return node;
}

// 構文解析4
// expr = assign
Node *expr(){
    return assign();
}

// 構文解析5
// assign = equality ("=" assign)?
Node *assign(){
    Node *node = equality();
    if(consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    } else {
        return node;
    }
    return node;
}

// 構文解析6
// equality = relational ("==" relational | "!=" relational)*
Node *equality(){
    Node *node = relational();

    if(consume("==")) {
        node = new_node(ND_EQ, node, relational());
    } else if(consume("!=")) {
        node = new_node(ND_NEQ, node, relational());
    } else {
        return node;
    }

    define_type(&node->type, INT);
    return node;
}

// 構文解析7
// relational = add (">" add | ">=" add | "<" add | "<=" add)*
Node *relational(){
    Node *node = add();

    // <, <= は両辺入れ替えて >, >= と同じように扱う(発想の勝利)
    if(consume(">")) {
        node = new_node(ND_UPPERL, node, add());
    } else if(consume(">=")) {
        node = new_node(ND_UPPEREQL, node, add());
    } else if(consume("<")) {
        node = new_node(ND_UPPERL, add(), node);
    } else if(consume("<=")) {
        node = new_node(ND_UPPEREQL, add(), node);
    } else {
        return node;
    }

    define_type(&node->type, INT);
    return node;
}

// 構文解析8
// add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();

    while(true) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
            define_type(&node->type, max_type(node->left->type, node->right->type)->ty);
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
            define_type(&node->type, max_type(node->left->type, node->right->type)->ty);
        } else {
            return node;
        }
    }
}

// 構文解析9
// mul = unary ("*" unary | "-" unary)*
Node *mul(){
    Node *node = unary();

    while(true) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
            define_type(&node->type, max_type(node->left->type, node->right->type)->ty);
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
            define_type(&node->type, max_type(node->left->type, node->right->type)->ty);
        } else if(consume("%")) {
            node = new_node(ND_DIV_REMAIN, node, unary());
            define_type(&node->type, max_type(node->left->type, node->right->type)->ty);
        } else {
            return node;
        }
    }
}

// 構文解析10
// unary = "sizeof" unary | ("+" | "-")? primary | ("*" | "&") unary | unary "[" unary "]"
Node *unary(){
    if(token->kind == TOKEN_SIZEOF) {
        token = token->next;
        Node *node = unary();
        int type = node->type->ty;
        free(node);
        return new_num_node(type_to_size(node->type));
    }

    if(consume("-")) {
        Node *node = new_node(ND_SUB, new_num_node(0), primary());
        define_type(&node->type, INT);
        return node;
    }

    Node *node = NULL;
    if(consume("*")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->left = unary();
        node->type = node->left->type->ptr_to;
        goto check_array_access;
    }

    if(consume("&")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->left = unary();
        define_type(&node->type, PTR);
        define_type(&node->type->ptr_to, PTR);
        goto check_array_access;
    }

    if(node == NULL) {
        node = primary();
        goto check_array_access;
    }

    // "[" (ident | num ) "]"
check_array_access:
    if(consume("[")) {
        Node *deref_par = calloc(1, sizeof(Node));
        Node *add = calloc(1, sizeof(Node));
        deref_par->kind = ND_DEREF;
        deref_par->left = add;
        add->kind = ND_ADD;
        add->left = node;
        add->right = expr();
        define_type(&add->type, max_type(add->left->type->ptr_to, add->right->type->ptr_to)->ty);
        define_type(&deref_par->type, add->type->ty);
        expect("]");
        return deref_par;
    }
    return node;
}

// 構文解析11
// primary = "(" expr ") | ident ("(" (expr ","?)* ")") | num | ident
Node *primary(){
    // "(" expr ")"
    if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *next_token = consume_ident();
    if(next_token){
        // 関数呼び出し
        if(consume("(")) {
            // 関数名
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_CALL_FUNC;
            node->f_name = (char*)malloc(next_token->len * sizeof(char));
            strncpy(node->f_name, next_token->str, next_token->len);
            node->f_name[next_token->len] = '\0';

            // 引数
            node->args = calloc(6, sizeof(Node));
            for(int idx = 0; idx < 6; ++ idx) {
                Token *bef_token = token;
                (   consume("*") ||
                    consume("&") ||
                    (consume_kind(TOKEN_SIZEOF) && consume("("))
                );
                if(consume_number() || consume_ident()){
                    token = bef_token;
                    node->args[idx] = expr();
                }
                if(!consume(",")){
                    break;
                }
            }
            expect(")");
            return node;
        }

        // 変数
        Node *node = calloc(1, sizeof(Node));
        LVar *result = find_lvar(next_token);
        node->offset = result->offset;
        node->type = result->type;
        node->kind = ND_LVER;

        // 変数が配列を指していた場合、先頭アドレスへのポインタに変換する
        if(node->type->ty == ARRAY) {
            free(node);
            Node *addr_par = calloc(1, sizeof(Node));
            Node *addr = calloc(1, sizeof(Node));
            addr_par->kind = ND_ADDR;
            addr_par->left = addr;
            addr->kind = ND_LVER;
            addr->offset = result->offset;
            define_type(&addr_par->type, PTR);
            define_type(&addr_par->type->ptr_to, INT);
            node = addr_par;
        }
        return node;
    }

    return new_num_node(expect_number());
}

// ローカル変数検索
LVar *find_lvar(Token *request){
    // 検索
    for(LVar *var = locals; var; var = var->next){
        if(var->len == request->len && strncmp(var->name, request->str, request->len) == 0){
            return var;
        }
    }

    char *name = (char*)malloc(request->len*sizeof(char));
    strncpy(name, request->str, request->len);
    error("[ERROR] 定義されていない変数です => %s\n", name);
}

// 変数登録
LVar *regist_var(bool is_global){
    // "int"
    Type *int_type = calloc(1, sizeof(Type));
    int_type->ty = INT;
    int_type->ptr_to = NULL;
    if(!consume_kind(TOKEN_INT)) {
        return NULL;
    }

    // "*"*
    Type *type = int_type;
    while(consume("*")) {
        Type *ptr_type = calloc(1, sizeof(Type));
        ptr_type->ty = PTR;
        ptr_type->ptr_to = type;
        type = ptr_type;
    }

    // 変数名
    GVar *gvar;
    LVar *lvar;
    if(is_global) {
        gvar = calloc(1, sizeof(GVar));
        Token *var_name = consume_ident();
        gvar->type = type;
        gvar->next = globals;
        gvar->name = var_name->str;
        gvar->len = var_name->len;
        globals = gvar;
    } else {
        lvar = calloc(1, sizeof(LVar));
        Token *var_name = consume_ident();
        lvar->type = type;
        lvar->next = locals;
        lvar->name = var_name->str;
        lvar->len = var_name->len;
        lvar->offset = locals->offset + 8;
        locals = lvar;
    }

    // "[" array_size "]"
    if(consume("[")) {
        size_t size = expect_number();
        int_type->ty = ARRAY;
        int_type->size = size;
        if(!is_global) lvar->offset = locals->offset - 8 + (8 * size);
        expect("]");
        if(size <= 0) {
            error("[ERROR] 長さが0以下の配列は定義できません");
        }
    }
    return lvar;
}