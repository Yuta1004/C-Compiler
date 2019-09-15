#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yncc.h"


// 左辺値コンパイル
void gen_lval(Node *node){
    if(node->kind != ND_LVER){
        error("左辺値が変数ではありません");
    }

    printf("        mov rax, rbp\n");
    printf("        sub rax, %d\n", node->offset);
    printf("        push rax\n");
    return;
}

// 構文木 to アセンブリ
void gen_asm(Node *node){
    if(node == NULL) return;
    int tmp_label_numbers = label_numbers;

    // 変数, 値
    switch(node->kind){
    case ND_NUM:
        printf("        push %d\n", node->val);
        return;
    case ND_LVER:   // 右辺に左辺値が出てきた場合
        gen_lval(node);
        printf("        pop rax\n");
        printf("        mov rax, [rax]\n");
        printf("        push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->left);                   // [a] = 9 + 1  : LEFT
        gen_asm(node->right);                   // a = [9 + 1]  : RIGHT
        printf("        pop rdi\n");            // RIGHT
        printf("        pop rax\n");            // LEFT
        printf("        mov [rax], rdi\n");     // [LEFT] = RIGHT
        printf("        push rdi\n");           // a=b=c=8 が出来るように右辺値をスタックに残しておく
        return;
    }

    // 予約語
    switch(node->kind){
    case ND_RETURN:
        gen_asm(node->left);
        printf("        pop rax\n");
        printf("        mov rsp, rbp\n");
        printf("        pop rbp\n");
        printf("        ret\n");
        return;
    case ND_IF:
        label_numbers ++;
        gen_asm(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 1\n");
        printf("        jne .L__if_else_%d\n", tmp_label_numbers);
        gen_asm(node->right->left);
        printf("        jmp .L__if_end_%d\n", tmp_label_numbers);
        printf(".L__if_else_%d:\n", tmp_label_numbers);
        gen_asm(node->right->right);
        printf(".L__if_end_%d:\n", tmp_label_numbers);
        return;
    case ND_WHILE:
        label_numbers ++;
        printf(".L__while_start_%d:\n", tmp_label_numbers);
        gen_asm(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 1\n");
        printf("        jne .L__while_end_%d\n", tmp_label_numbers);
        gen_asm(node->right);
        printf("        jmp .L__while_start_%d\n", tmp_label_numbers);
        printf(".L__while_end_%d:\n", tmp_label_numbers);
        return;
    }

    gen_asm(node->left);
    gen_asm(node->right);

    printf("        pop rdi\n");
    printf("        pop rax\n");

    switch(node->kind){
    case ND_ADD:
        printf("        add rax, rdi\n");
        break;
    case ND_SUB:
        printf("        sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("        imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("        cqo\n");        // raxレジスタをrdxと合わせた128bitに拡張
        printf("        idiv rdi\n");   // rax / rsiの結果 (余りはrdx)
        break;
    case ND_EQ:
        printf("        cmp rdi, rax\n");   // rdiとraxを比較 -> 結果はフラグレジスタへ
        printf("        sete al\n");        // 比較結果(==)をalに入れる(raxの下位8ビットにあたるレジスタ)
        printf("        movzb rax, al\n");  // raxレジスタの上位56ビットをゼロクリア
        break;
    case ND_NEQ:
        printf("        cmp rdi, rax\n");
        printf("        setne al\n");       // 比較結果(!=)をalに入れる
        printf("        movzb rax, al\n");
        break;
    case ND_UPPERL:
        printf("        cmp rdi, rax\n");
        printf("        setl al\n");        // 比較結果(>)をalに入れる
        printf("        movzb rax, al\n");
        break;
    case ND_UPPEREQL:
        printf("        cmp rdi, rax\n");
        printf("        setle al\n");       // 比較結果(>=)をalに入れる
        printf("        movzb rax, al\n");
        break;
    default:
        error("[ERROR] 構文木解析エラー");
    }

    printf("        push rax\n");
}