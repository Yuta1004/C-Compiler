int failed_cnt = 0;

int try(int req, int expr) {
    printf("%d == %d\n", req, expr);
    if(req == expr) {
        printf("Success!!\n");
    } else {
        printf("Failed... : expect( %d ), output( %d )\n", req, expr);
        failed_cnt = failed_cnt + 1;
    }
    return 0;
}

int funcA(int num){
    return num;
}

int funcB(int a, int b, int c, int d){
    return a + b + c + d;
}

int funcC(int a, int b, int c, int d, int e){
    return a + b + c + d + e;
}

int main(){
    /* Expression */
    printf("Exprssion\n");
    try(0, ({ 0; }));
    try(10, ({ 1+2+3+4; }));
    try(30, ({ 10*(2+3+4+5+6)/5-(3+7); }));
    try(1, ({ 20*100 < 30000; }));

    /* Variable */
    printf("\nVariable\n");
    try(20, ({ int a; int b; int c; int d; int e; a = 10; b = 20; c = a*b; d = (c+100)/(e=10); d-e; }));
    try(200, ({ int a_result; int b_result; int ret_value; a_result = 1204; b_result = 1004; ret_value = a_result - b_result; ret_value; }));
    try(10, ({ int a; int b; a=b=10; }));

    /* If */
    printf("\nIf\n");
    try(100, ({ int a; int b; int c; a = 10; b = 10; if(a != b) c = 20; else c = 100; c; }));
    try(20, ({ int result; result = 0; if (1) if (1) if (0) result = result + 10; else if(1) result = result + 20; else result = result + 30; else result = result + 40; result; }));

    /* While */
    printf("\nWhile\n");
    try(10, ({ int num; num = 0; while(num < 10) num = num + 1; num; }));

    /* For */
    printf("\nFor\n");
    try(100, ({ int num; num = 0; for(; num < 100; ) num = num + 1; num; }));
    try(110, ({ int sum; int n; sum = 0; for(n = 1; n <= 20; n = n + 1) if(n % 2 == 0) sum = sum + n; sum; }));

    /* Call Function */
    printf("\nCall function\n");
    try(9, ({ funcA(9); }));
    try(100, ({ funcB(10, 20, 30, 40); }));
    try(341, ({ funcC(1, 4, 16, 64, 256); }));

    /* Define Function */
    printf("\nDefine function\n");
    printf("Success!!\n");

    /* Pointer */
    printf("\nPointer\n");
    try(3, ({ int x; int y; int *z; int zc; x = 3; y = 5; z = &y + 1; zc = *z; zc; }));
    try(10, ({ int x; int *l; int **z; x = 100; l = &x; z = &l; **z = 10; x; }));

    /* Sizeof */
    printf("\nSizeof\n");
    try(4, ({ sizeof(1); }));
    try(8, ({ int *a; sizeof(a); }));
    try(1, ({ char ca; sizeof(ca); }));

    /* Array */
    printf("\nArray\n");
    try(90, ({ int array[10]; int idx; int num; for(idx = 0; idx < 10; idx = idx + 1){ *(array + idx) = idx * 10; } for(idx = 0; idx < 10; idx = idx + 1){ num = *(array + idx); } num; }));
    try(30, ({ int array[2]; array[0] = 50; 1[array] = 20; 0[array] - array[1]; }));
    try(55, ({ int array[10]; array[0] = 1; array[1] = 1;int idx; for(idx = 2; idx < 10; idx = idx + 1){ array[idx] = array[idx-1] + array[idx-2]; } array[9]; }));

    /* Char */
    printf("\nChar\n");
    try(3, ({ char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; x[0] + y; }));
    try(0, ({ char a; char b; a = 128; b = -129; a + b + 1; }));

    /* Str */
    printf("\nStr\n");
    printf("Success!!\n");

    /* Comment */
    printf("\nComment\n");
    try(0, ({ /* abcdefghijklmn */ int a; a = 20; a - 20; }));

    /* For-ext */
    printf("\nFor-ext\n");
    try(55, ({int sum = 0; for(int i = 0; i <= 10; i = i+1) sum = sum+i; sum; }));

    /* Inc */
    printf("\nInc\n");
    try(10, ({ int a = 0; for(int i = 0; i < 10; ++ i) ++ a; a; }));
    try(5, ({ int a = 2; int b = a ++; a+b; }));
    try(8, ({ int a = 3; int b = ++ a; a+b; }));

    /* Dec */
    printf("\nDec\n");
    try(0, ({ int num = 10; for(int i = 10; i > 0; --i) num --; num; }));
    try(7, ({ int a = 4; int b = a --; a+b; }));
    try(100, ({ int a = 51; int b = --a; a+b; }));

    /* Test Result */
    if(failed_cnt == 0) {
        printf("\nALL Success!!\n");
        return 0;
    } else {
        printf("\nFailed... : %d\n", failed_cnt);
        return 1;
    }
}
