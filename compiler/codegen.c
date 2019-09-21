#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yncc.h"


/* 計算途中で使用するレジスタ */
static char *regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
static char *regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};
static char *regs32[] = {"r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d"};

/* 引数用のレジスタ */
static char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};


// 左辺値コンパイル
void gen_lval(Node *node){
    if(node->kind == ND_LVER) {
        printf("        mov rax, rbp\n");
        printf("        sub rax, %d\n", node->offset);
        printf("        push rax\n");
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
        printf("        pop rax\n");
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
        printf("        push %d\n", val);
        return;

    case ND_LVER:   // 右辺に左辺値が出てきた場合
        gen_lval(node);
        printf("        pop rax\n");
        printf("        mov rax, [rax]\n");
        printf("        push rax\n");
        return;

    case ND_ASSIGN:
        gen_lval(left);                         // [a] = 9 + 1  : LEFT
        gen_asm(right);                         // a = [9 + 1]  : RIGHT
        printf("        pop rbx\n");            // RIGHT
        printf("        pop rax\n");            // LEFT
        printf("        mov [rax], rbx\n");     // [LEFT] = RIGHT
        printf("        push rbx\n");           // a=b=c=8 が出来るように右辺値をスタックに残しておく
        return;

    case ND_BLOCK:;
        Node *block_node = node->block_next_node;   // ブロック連結リストのノードを持つ
        while(block_node != NULL) {
            gen_asm_with_pop(block_node);
            block_node = block_node->block_next_node;
        }
        printf("        push rax\n");
        return;

    case ND_FUNC:{
        printf("\n");
        printf("%s:\n", node->f_name);
        printf("        push rbx\n");
        printf("        push rbp\n");
        printf("        mov rbp, rsp\n");
        printf("        push r12\n");
        printf("        push r13\n");
        printf("        push r14\n");
        printf("        push r15\n");
        printf("        sub rsp, %d\n", 8*20);
        for(int idx = 0; idx < 6; ++ idx) {
            if(node->args[idx]){
                printf("        mov [rbp-%d], %s\n", idx*8+8, argregs[idx]);
            }
        }
        gen_asm_with_pop(left);
        printf("        mov rax, 0\n");
        printf("        pop r15\n");
        printf("        pop r14\n");
        printf("        pop r13\n");
        printf("        pop r12\n");
        printf("        mov rsp, rbp\n");
        printf("        pop rbp\n");
        printf("        pop rbx\n");
        printf("        ret\n\n");
        return;
    }

    case ND_CALL_FUNC:{
        printf("        push rdi\n");                                       // rdi, rsi
        printf("        push rsi\n");
        for(int idx = 0; idx < 6; ++ idx) {
            if(node->args[idx]) {
                gen_asm_with_pop(node->args[idx]);
                printf("        mov %s, rax\n", argregs[idx]);
            }
        }
        printf("        mov rbx, rsp\n");
        printf("        and rsp, 0xfffffffffffffff0\n");                    // アライメント
        printf("        call %s\n", node->f_name);
        printf("        mov rsp, rbx\n");
        printf("        pop rsi\n");
        printf("        pop rdi\n");
        printf("        push rax\n");
        return;
    }

    case ND_ADDR:
        gen_lval(left);
        return;

    case ND_DEREF:
        gen_asm(left);
        printf("        pop rax\n");
        printf("        mov rax, [rax]\n");
        printf("        push rax\n");
        return;
    }

    // 予約語
    switch(node->kind){
    case ND_RETURN:
        gen_asm_with_pop(left);
        printf("        mov rsp, rbp\n");
        printf("        pop rbp\n");
        printf("        pop rbx\n");
        printf("        ret\n");
        return;

    case ND_IF:
        label ++;
        gen_asm_with_pop(left);
        printf("        cmp rax, 1\n");
        printf("        jne .L__if_else_%d\n", tmp_label);
        gen_asm_with_pop(right->left);
        printf("        jmp .L__if_end_%d\n", tmp_label);
        printf(".L__if_else_%d:\n", tmp_label);
        gen_asm_with_pop(right->right);
        printf(".L__if_end_%d:\n", tmp_label);
        printf("        push 0\n");
        return;

    case ND_WHILE:
        label ++;
        printf(".L__while_start_%d:\n", tmp_label);
        gen_asm_with_pop(left);
        printf("        cmp rax, 1\n");
        printf("        jne .L__while_end_%d\n", tmp_label);
        gen_asm_with_pop(right);
        printf("        jmp .L__while_start_%d\n", tmp_label);
        printf(".L__while_end_%d:\n", tmp_label);
        printf("        push 0\n");
        return;

    case ND_FOR:
        label ++;
        gen_asm_with_pop(left);
        printf(".L__for_start_%d:\n", tmp_label);
        gen_asm_with_pop(right->left->left);
        printf("        cmp rax, 1\n");
        printf("        jne .L__for_end_%d\n", tmp_label);
        gen_asm_with_pop(right->left->right);
        gen_asm_with_pop(right->right);
        printf("        jmp .L__for_start_%d\n", tmp_label);
        printf(".L__for_end_%d:\n", tmp_label);
        printf("        push 0\n");
        return;
    }

    gen_asm(left);
    gen_asm(right);

    printf("        pop rbx\n");
    printf("        pop rax\n");

    bool is_left_ptr = (left->type != NULL && left->type->ty == PTR);
    bool is_right_ptr = (right->type != NULL && right->type->ty == PTR);

    // 式
    switch(node->kind){
    case ND_ADD:
        if(is_left_ptr)
            printf("        imul rbx, %d\n", type_to_size(left->type->ptr_to));
        if(is_right_ptr)
            printf("        imul rax, %d\n", type_to_size(right->type->ptr_to));
        printf("        add rax, rbx\n");
        break;

    case ND_SUB:
        if(is_left_ptr)
            printf("        imul rbx, %d\n", type_to_size(left->type->ptr_to));
        if(is_right_ptr)
            printf("        imul rax, %d\n", type_to_size(right->type->ptr_to));
        printf("        sub rax, rbx\n");
        break;

    case ND_MUL:
        printf("        imul rax, rbx\n");
        break;

    case ND_DIV:
        printf("        cqo\n");        // raxレジスタをrdxと合わせた128bitに拡張
        printf("        idiv rbx\n");   // rax / rsiの結果 (余りはrdx)
        break;

    case ND_DIV_REMAIN:
        printf("        cqo\n");
        printf("        idiv rbx\n");
        printf("        mov rax, rdx\n");
        break;

    case ND_EQ:
        printf("        cmp rbx, rax\n");   // rdiとraxを比較 -> 結果はフラグレジスタへ
        printf("        sete al\n");        // 比較結果(==)をalに入れる(raxの下位8ビットにあたるレジスタ)
        printf("        movzb rax, al\n");  // raxレジスタの上位56ビットをゼロクリア
        break;

    case ND_NEQ:
        printf("        cmp rbx, rax\n");
        printf("        setne al\n");       // 比較結果(!=)をalに入れる
        printf("        movzb rax, al\n");
        break;

    case ND_UPPERL:
        printf("        cmp rbx, rax\n");
        printf("        setl al\n");        // 比較結果(>)をalに入れる
        printf("        movzb rax, al\n");
        break;

    case ND_UPPEREQL:
        printf("        cmp rbx, rax\n");
        printf("        setle al\n");       // 比較結果(>=)をalに入れる
        printf("        movzb rax, al\n");
        break;

    default:
        error("[ERROR] 構文木解析エラー");
    }

    printf("        push rax\n");
}
