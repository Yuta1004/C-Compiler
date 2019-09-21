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

// ラベル出力
void outlabel(char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    va_end(va);
    printf(":\n");
}

// アセンブリ出力
void outasm(char *fmt, ...) {
    printf("\t\t");
    va_list va;
    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    va_end(va);
    printf("\n");
}

// 左辺値コンパイル
void gen_lval(Node *node){
    if(node->kind == ND_VAR) {
        outasm("mov rax, rbp");
        outasm("sub rax, %d", node->offset);
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
    if(node == NULL) return;
    int tmp_label = label;

    int val = node->val;
    Node *left = node->left;
    Node *right = node->right;

    // 変数, 値, ブロック, 関数定義, 関数呼び出し, ポインタ
    switch(node->kind){
    case ND_NUM:
        outasm("push %d", val);
        return;

    case ND_VAR:   // 右辺に左辺値が出てきた場合
        gen_lval(node);
        outasm("pop rax");
        outasm("mov %s, %s [rax]", reg(0, node->type), size_stmt(node->type));
        outasm("push rax");
        return;

    case ND_ASSIGN:
        gen_lval(left);                                     // [a] = 9 + 1  : LEFT
        gen_asm(right);                                     // a = [9 + 1]  : RIGHT
        outasm("pop %s", regs[1]);                          // RIGHT
        outasm("pop rax");                                  // LEFT
        outasm("mov %s [rax], %s", size_stmt(right->type), reg(1, right->type));
        outasm("push %s", regs[1]);                         // a=b=c=8 が出来るように右辺値をスタックに残しておく
        return;

    case ND_BLOCK:;
        Node *block_node = node->block_next_node;   // ブロック連結リストのノードを持つ
        while(block_node != NULL) {
            gen_asm_with_pop(block_node);
            block_node = block_node->block_next_node;
        }
        outasm("push rax");
        return;

    case ND_DEFGVAR:
        outlabel("%s", node->name);
        outasm(".zero %d", type_to_size(node->type) * node->type->size);
        printf("\n");
        return;

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

    case ND_DEREF:
        gen_asm(left);
        outasm("pop rax");
        outasm("mov rax, [rax]");
        outasm("push rax");
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
        label ++;
        // 条件式
        gen_asm_with_pop(left);
        outasm("cmp rax, 1");
        outasm("jne .L__if_else_%d", tmp_label);
        // 処理<true>
        gen_asm_with_pop(right->left);
        outasm("jmp .L__if_end_%d", tmp_label);
        // 処理<false>
        outlabel(".L__if_else_%d", tmp_label);
        gen_asm_with_pop(right->right);
        outlabel(".L__if_end_%d", tmp_label);
        outasm("push 0");
        return;

    case ND_WHILE:
        label ++;
        outlabel(".L__while_start_%d", tmp_label);
        // 条件式
        gen_asm_with_pop(left);
        outasm("cmp rax, 1");
        outasm("jne .L__while_end_%d", tmp_label);
        // 処理
        gen_asm_with_pop(right);
        outasm("jmp .L__while_start_%d", tmp_label);
        outlabel(".L__while_end_%d", tmp_label);
        outasm("push 0");
        return;

    case ND_FOR:
        label ++;
        // 初期化
        gen_asm_with_pop(left);
        outlabel(".L__for_start_%d", tmp_label);
        // 条件式
        gen_asm_with_pop(right->left->left);
        outasm("cmp rax, 1");
        outasm("jne .L__for_end_%d", tmp_label);
        // 処理
        gen_asm_with_pop(right->left->right);
        // 変化式
        gen_asm_with_pop(right->right);
        outasm("jmp .L__for_start_%d", tmp_label);
        outlabel(".L__for_end_%d", tmp_label);
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
