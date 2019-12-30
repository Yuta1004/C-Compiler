#include <stdio.h>
#include <string.h>
#include "yncc.h"

// 構造体定義
// "{" (type ident ("," type ident)*)? "}"
// structキーワード, タグ名は既にパースされた前提
bool regist_struct(char *tag) {
    // {
    if(!consume("{"))
        return false;

    fprintf(stderr, "%s\n", tag);

    // (type ident ("," type ident)*)?
    int offset = 0;
    while(true) {
        // type ident
        Type *type = read_type();
        if(!type) break;
        Token *ident = expect_ident();
        if(!ident) break;
        expect(";");
    }

    expect("}");
    return true;
}