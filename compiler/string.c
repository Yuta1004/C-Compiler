#include <string.h>
#include "yncc.h"

int find_str(char *target_str) {
    for(int idx = 0; idx < str_vec->len; ++ idx) {
        char *str = (char*)vec_get(str_vec, idx);
        if(strcmp(target_str, str) == 0) {
            return idx;
        }
    }
}