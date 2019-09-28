#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "yncc.h"

char *read_file(char *filepath) {
    // ファイルオープン
    FILE *fp = fopen(filepath, "r");
    if(!fp) {
        error("Cannot open file %s: %s\n", filepath, strerror(errno));
    }

    // ファイルの長さを求める
    if(fseek(fp, 0, SEEK_END) == -1) {
        error("fseek error %s: %s\n", filepath, strerror(errno));
    }
    size_t size = ftell(fp);
    if(fseek(fp, 0, SEEK_SET) == -1) {
        error("fseek error %s: %s\n", filepath, strerror(errno));
    }

    // ファイル読み込み
    char *fbuf = (char*)calloc(1, size + 2);
    fread(fbuf, size, 1, fp);

    // ファイル終端が必ず\n\0となるようにする
    if(size == 0 || fbuf[size - 1] != '\n') {
        fbuf[size++] = '\n';
    }
    fbuf[size] = '\0';
    fclose(fp);

    return fbuf;
}
