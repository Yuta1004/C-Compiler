#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yncc.h"

// 構文木 to アセンブリ
void gen_asm(Node *node){
    if(node->kind == ND_NUM) {
        printf("        push %d\n", node->val);
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