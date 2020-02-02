#include "../yncc.h"
#include "parse.h"

// program = func*
void program(){
    while(!at_eof())
        vec_push(codes, func());
}