#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "yncc.h"

// エラー出力関数
void error(char *fmt, ...){
    va_list vargs;
    va_start(vargs, fmt);
    vfprintf(stderr, fmt, vargs);
    fprintf(stderr, "\n");
    exit(1);
}

//  エラー出力関数(詳細版)
void error_at(char *location, char *fmt, ...){
    va_list vargs;
    va_start(vargs, fmt);

    int err_pos = location - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", err_pos, "");
    fprintf(stderr, "^\t");
    vfprintf(stderr, fmt, vargs);
    fprintf(stderr, "\n\n");
    exit(1);
}