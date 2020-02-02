#include "../yncc.h"
#include "parse.h"

// func = type ident "(" (type ident ","?)* ")" block
//      | type ident "*"* ("[" num "]")? ("=" equality)? ";"
//      | "typedef" type ";"
Node *func(){
    Token *bef_token = token;

    // typedef
    if(consume_kind(TOKEN_TYPEDEF)) {
        Type *type = read_type();
        do_typedef(token->str, token->len, type);
        token = token->next;
        expect(";");
        return NULL;
    }

    // ローカル変数初期化
    Node *node = new_node(ND_FUNC);
    vec_free(locals);
    locals = vec_new(10);
    sum_offset = 0;

    // 構造体(グローバル)
    if(consume_kind(TOKEN_STRUCT)) {
        Token *tag = expect_ident();
        if(def_struct(GLOBAL, tag->str, tag->len)) {
            expect(";");
            return new_none_node();
        }
        token = bef_token;
    }

    // 関数名
    read_type();
    Token *f_name_token = consume_ident();
    if(!f_name_token){
        error_at(token->str, "関数定義が要求されました");
    }
    node->name = (char*)calloc(f_name_token->len+1, sizeof(char));
    strncpy(node->name, f_name_token->str, f_name_token->len);

    // 関数定義
    if(consume("(")) {
        in_scope();
        node->args = calloc(6, sizeof(Node));
        for(int idx = 0; idx < 6; ++ idx) {
            Var *var = regist_var(LOCAL);
            if(var) {                                      // 引数があるかチェック
                Node *arg_node = new_node(ND_LVAR);
                arg_node->offset = var->offset;
                node->args[idx] = arg_node;
            }
            if(!consume(",")){                              // , が無ければ続く引数は無いと判断する
                break;
            }
        }
        expect(")");
        node->left = block();
        out_scope();
        return node;
    }

    // グローバル変数定義
    token = bef_token;
    Var *var = regist_var(GLOBAL);
    var->init_expr = new_none_node();
    var->init_expr->type = var->type;
    if(consume("=")) {
        free(var->init_expr);
        var->init_expr = expr();
    }
    expect(";");
    return new_none_node();
}