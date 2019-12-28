#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "yncc.h"


/* 計算途中で使用するレジスタ */
static char *regs[] = {"rax", "r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
static char *regs8[] = {"al", "r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};
static char *regs32[] = {"eax", "r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d"};

/* 引数用のレジスタ */
static char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// 番号, 型に対して適切なレジスタ名を返す
char *reg(int id, Type *type) {
    switch(type_to_size(type)) {
    case 1:
        return regs8[id];
    case 4:
        return regs32[id];
    default:
        return regs[id];
    }
}

// 型に対して適切なサイズ指定文を返す
char* size_stmt(Type *type) {
    switch(type_to_size(type)) {
    case 1:
        return "byte ptr";
    case 4:
        return "dword ptr";
    default:
        return "qword ptr";
    }
}

// 左辺値コンパイル
void gen_lval(Node *node){
    if(node->kind == ND_LVAR) {
        outasm("mov rax, rbp");
        outasm("sub rax, %d", node->offset);
        outasm("push rax");
        return;
    }
    if(node->kind == ND_GVAR) {
        outasm("lea rax, %s[rip]", node->name);
        outasm("push rax");
        return;
    }
    if(node->kind == ND_DEREF) {
        gen_asm(node->left);
        return;
    }
    error("左辺値が変数ではありません");
}

// 構文木 to アセンブリ (with <pop rax>)
void gen_asm(Node *node);
void gen_asm_with_pop(Node *node) {
    gen_asm(node);
    if(node != NULL) {
        outasm("pop rax");
    }
}

// 構文木 to アセンブリ
void gen_asm(Node *node){
    if(node == NULL || node->kind == ND_NONE) return;

    int val = node->val, tmp_label;
    Node *left = node->left;
    Node *right = node->right;

    // 変数, 値, ブロック, 関数定義, 関数呼び出し, ポインタ
    switch(node->kind){
    case ND_NUM:
        outasm("push %d", val);
        return;

    case ND_STR:
        outasm("mov rax, OFFSET FLAT:.str%d", val);
        outasm("push rax");
        return;

    case ND_GVAR:
        // 文字列をもつグローバル変数
        if(node->type->ty == STR)
            return;

    case ND_LVAR:
        gen_lval(node);

    case ND_DEREF:
        if(node->kind == ND_DEREF)
            gen_asm(left);

        // ** GVAR, LVAR, DEREF共通処理 **
        // 型に合わせてmov命令
        outasm("pop rax");
        if(type_to_size(node->type) == 1)
            outasm("movsx eax, %s [rax]", size_stmt(node->type));
        else
            outasm("mov %s, %s [rax]", reg(0, node->type), size_stmt(node->type));
        outasm("push rax");
        return;

    case ND_INIT_GVAR:
        outlabel("%s", left->name, right->kind);
        switch(right->kind){
        case ND_NONE:
            outasm(".zero %d", type_to_size(right->type) * right->type->size);
            return;

        case ND_STR:
            outasm(".ascii \"%s\\0\"", (char*)vec_get(str_vec, right->val));
            return;

        case ND_INIT_ARRAY:{
            int size = (left->type->size > right->val) ? left->type->size : right->val;
            for(int idx = 0; idx < size; ++ idx) {
                if(idx < right->val)
                    outasm(".long %d", precalc_expr((Node*)vec_get(right->node_list, idx)));
                else
                    outasm(".long 0");
            }
            return;
        }

        default:
            if(right->type->ty == PTR) {
                char *expr = malloc(10*sizeof(char));
                decode_precalc_expr(expr, right);
                outasm(".quad %s", expr);
                return;
            }
            outasm(".long %d", precalc_expr(right));
            return;
        }

    case ND_ASSIGN:
        gen_lval(left);                                     // [a] = 9 + 1  : LEFT
        gen_asm(right);                                     // a = [9 + 1]  : RIGHT
        outasm("pop %s", regs[1]);                          // RIGHT
        outasm("pop rax");                                  // LEFT
        outasm("mov %s [rax], %s", size_stmt(left->type), reg(1, left->type));
        outasm("push %s", regs[1]);                         // a=b=c=8 が出来るように右辺値をスタックに残しておく
        return;

    case ND_BLOCK:{
        for(int idx = 0; idx < node->node_list->len; ++ idx) {
            gen_asm_with_pop((Node*)vec_get(node->node_list, idx));
        }
        outasm("push rax");
        return;
    }

    case ND_FUNC:{
        printf("\n");
        printf("%s:\n", node->name);
        outasm("push rbx");
        outasm("push rbp");
        outasm("mov rbp, rsp");
        outasm("push r12");
        outasm("push r13");
        outasm("push r14");
        outasm("push r15");
        outasm("sub rsp, %d", 8*20);

        for(int idx = 0; idx < 6 && node->args[idx]; ++ idx)
            outasm("mov [rbp-%d], %s", idx*8+8, argregs[idx]);
        gen_asm_with_pop(left);

        outasm("mov rax, 0");
        outasm("pop r15");
        outasm("pop r14");
        outasm("pop r13");
        outasm("pop r12");
        outasm("mov rsp, rbp");
        outasm("pop rbp");
        outasm("pop rbx");
        outasm("ret");
        printf("\n");
        return;
    }

    case ND_CALL_FUNC:{
        outasm("push rdi");                                       // rdi, rsi
        outasm("push rsi");
        for(int idx = 0; idx < 6 && node->args[idx]; ++ idx) {
            gen_asm_with_pop(node->args[idx]);
            outasm("mov %s, rax", argregs[idx]);
        }
        outasm("mov al, 0");
        outasm("mov rbx, rsp");
        outasm("and rsp, 0xfffffffffffffff0");                    // アライメント
        outasm("call %s", node->name);
        outasm("mov rsp, rbx");
        outasm("pop rsi");
        outasm("pop rdi");
        outasm("push rax");
        return;
    }

    case ND_ADDR:
        gen_lval(left);
        return;

    case ND_PRE_INC:
        gen_lval(left);
        outasm("pop rax");
        outasm("inc %s [rax]", size_stmt(left->type));
        outasm("mov rax, [rax]");
        outasm("push rax");
        return;

    case ND_PRE_DEC:
        gen_lval(left);
        outasm("pop rax");
        outasm("dec %s [rax]", size_stmt(left->type));
        outasm("mov rax, [rax]");
        outasm("push rax");
        return;

    case ND_POST_INC:
        gen_lval(left);
        outasm("pop rax");
        outasm("mov %s, [rax]", regs[1]);
        outasm("push %s", regs[1]);
        outasm("inc %s [rax]", size_stmt(left->type));
        return;

    case ND_POST_DEC:
        gen_lval(left);
        outasm("pop rax");
        outasm("mov %s, [rax]", regs[1]);
        outasm("push %s", regs[1]);
        outasm("dec %s [rax]", size_stmt(left->type));
        return;
    }

    // 予約語
    switch(node->kind){
    case ND_RETURN:
        gen_asm_with_pop(left);
        outasm("pop r15");
        outasm("pop r14");
        outasm("pop r13");
        outasm("pop r12");
        outasm("mov rsp, rbp");
        outasm("pop rbp");
        outasm("pop rbx");
        outasm("ret");
        return;

    case ND_IF:
        tmp_label = ++ label_if;
        // 条件式
        gen_asm_with_pop(left);
        outasm("cmp rax, 1");
        outasm("jne .L_if_else_%d", tmp_label);
        // 処理<true>
        gen_asm_with_pop(right->left);
        outasm("jmp .L_if_fin_%d", tmp_label);
        // 処理<false>
        outlabel(".L_if_else_%d", tmp_label);
        gen_asm_with_pop(right->right);
        outlabel(".L_if_fin_%d", tmp_label);
        outasm("push 0");
        return;

    case ND_WHILE:
        tmp_label = ++ label_loop;
        outlabel(".L_loop_continue_%d", tmp_label);
        // 条件式
        gen_asm_with_pop(left);
        outasm("cmp rax, 1");
        outasm("jne .L_loop_break_%d", tmp_label);
        // 処理
        gen_asm_with_pop(right);
        outasm("jmp .L_loop_continue_%d", tmp_label);
        outlabel(".L_loop_break_%d", tmp_label);
        outasm("push 0");
        return;

    case ND_FOR:
        tmp_label = ++ label_loop;
        // 初期化
        gen_asm_with_pop(left);
        outlabel(".L_loop_top_%d", tmp_label);
        // 条件式
        gen_asm_with_pop(right->left->left);
        outasm("cmp rax, 1");
        outasm("jne .L_loop_break_%d", tmp_label);
        // 処理
        gen_asm_with_pop(right->left->right);
        // 変化式
        outlabel(".L_loop_continue_%d", tmp_label);
        gen_asm_with_pop(right->right);
        outasm("jmp .L_loop_top_%d", tmp_label);
        outlabel(".L_loop_break_%d", tmp_label);
        outasm("push 0");
        return;
    }

    gen_asm(left);
    gen_asm(right);

    outasm("pop rbx");
    outasm("pop rax");

    bool is_left_ptr = (left->type != NULL && left->type->ty == PTR);
    bool is_right_ptr = (right->type != NULL && right->type->ty == PTR);

    // 式
    switch(node->kind){
    case ND_ADD:
        if(is_left_ptr)
            outasm("imul rbx, %d", type_to_size(left->type->ptr_to));
        if(is_right_ptr)
            outasm("imul rax, %d", type_to_size(right->type->ptr_to));
        outasm("add rax, rbx");
        break;

    case ND_SUB:
        if(is_left_ptr)
            outasm("imul rbx, %d", type_to_size(left->type->ptr_to));
        if(is_right_ptr)
            outasm("imul rax, %d", type_to_size(right->type->ptr_to));
        outasm("sub rax, rbx");
        break;

    case ND_MUL:
        outasm("imul rax, rbx");
        break;

    case ND_DIV:
        outasm("cqo");        // raxレジスタをrdxと合わせた128bitに拡張
        outasm("idiv rbx");   // rax / rsiの結果 (余りはrdx)
        break;

    case ND_DIV_REMAIN:
        outasm("cqo");
        outasm("idiv rbx");
        outasm("mov rax, rdx");
        break;

    case ND_EQ:
        outasm("cmp rbx, rax");   // rdiとraxを比較 -> 結果はフラグレジスタへ
        outasm("sete al");        // 比較結果(==)をalに入れる(raxの下位8ビットにあたるレジスタ)
        outasm("movzb rax, al");  // raxレジスタの上位56ビットをゼロクリア
        break;

    case ND_NEQ:
        outasm("cmp rbx, rax");
        outasm("setne al");       // 比較結果(!=)をalに入れる
        outasm("movzb rax, al");
        break;

    case ND_UPPERL:
        outasm("cmp rbx, rax");
        outasm("setl al");        // 比較結果(>)をalに入れる
        outasm("movzb rax, al");
        break;

    case ND_UPPEREQL:
        outasm("cmp rbx, rax");
        outasm("setle al");       // 比較結果(>=)をalに入れる
        outasm("movzb rax, al");
        break;

    default:
        error("[ERROR] 構文木解析エラー");
    }

    outasm("push rax");
}
