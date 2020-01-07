#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "yncc.h"

#define _vfprintf(out, fmt) va_list va; va_start(va, (fmt)); vfprintf((out), fmt, va); va_end(va);

// エラー出力関数
void error(char *fmt, ...){
    _vfprintf(stderr, fmt)
    fprintf(stderr, "\n");
    exit(1);
}

//  エラー出力関数(詳細版)
void error_at(char *location, char *fmt, ...){
    if(location == NULL){
        location = program_body + strlen(program_body);
    }

    // 対象となる行の最初と最後を見つける
    char *line = location;
    char *end = location;
    while(program_body < line && line[-1] != '\n') -- line;
    while(*end != '\n') ++ end;

    // 何行目か調べる
    int line_num = 1;
    for(char *p = program_body; p < line; ++ p) {
        if(*p == '\n')
            ++ line_num;
    }

    // 出力
    int indent = fprintf(stderr, "program:%d: ", line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);
    int pos = location - line + indent;
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    _vfprintf(stderr, fmt);
    fprintf(stderr, "\n");
    exit(1);
}

// 普通の出力
void outtxt(char *fmt, ...) {
    _vfprintf(stdout, fmt);
    printf("\n");
}

// ラベル出力
void outlabel(char *fmt, ...){
    _vfprintf(stdout, fmt);
    printf(":\n");
}

// アセンブリ出力
void outasm(char *fmt, ...) {
    printf("\t\t");
    _vfprintf(stdout, fmt);
    printf("\n");
}


// 文字がトークンを構成出来るか調べる
int is_alnum(char chr){
    return ('a' <= chr && chr <= 'z') ||
           ('A' <= chr && chr <= 'Z') ||
           ('1' <= chr && chr <= '9') ||
           (chr == '_');
}
