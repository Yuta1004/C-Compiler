#ifndef YNCC_H
#define YNCC_H

typedef struct Token Token;
typedef struct Node Node;
typedef struct Type Type;
typedef struct Var Var;
typedef struct Struct Struct;
typedef struct Typedef Typedef;

#include <stdbool.h>
#include <stdlib.h>

#include "vector.h"
#include "node/node.h"
#include "token/token.h"
#include "parse/parse.h"

#define LOCAL 0
#define GLOBAL 1
struct Var {
    int var_type;    // LOCAL or GLOBAL
    Var *next;       // 次のVar
    char *name;      // 変数名
    int len;         // 長さ
    int offset;      // ローカル変数だった時、RBPからのオフセット
    Type *type;      // 型
    Node *init_expr; // グローバル変数だった時、初期化式のノード
    int scope_id;     // ネストのID
};

struct Type {
    enum{ NONE, INT, PTR, ARRAY, CHAR, STR, STRUCT } ty;
    struct Type *ptr_to;
    char *tag;
    int len;
    int bytesize;
    int padsize;
    int scope_id;
};

struct Struct {
    char *tag;
    int len;            // tagの長さ
    int var_type;       // LOCAL or GLOBAL
    int bytesize;       // メモリサイズ
    int scope_id;       // スコープID
    Vector *members;     // メンバリスト Vector<Type>
    Vector *names;       // メンバ名一覧 Vector<char*>
};

struct Typedef {
    char *tag;
    int len;            // tagの長さ
    Type *type;
};

#include "node/node.h"
#include "token/token.h"
#include "parse/parse.h"

/* グローバル変数 */
Token *token;
char *program_body;
Vector *codes;
Vector *locals;
Vector *globals;
Vector *locals_struct;
Vector *globals_struct;
Vector *struct_def_list;
Vector *str_vec;
Vector *man_scope;
Vector *man_typedef;
int label, label_if, label_loop, sum_offset, scope_id, scope_sum_id;

/* common.c */
void error(char *fmt, ...);
void error_at(char *location, char *fmt, ...);
void outtxt(char *fmt, ...);
void outasm(char *fmt, ...);
void outlabel(char *fmt, ...);
bool is_alnum(char chr);
bool _strncmp(char *str1, char *str2, int str1_len, int str2_len);

/* type.c */
Type *new_type();
int type_to_size(Type *type);
void define_type(Type **type_ptr, int type);
void copy_type(Type **dst, Type *src);
Type *max_type(Type *a, Type *b);
Type *read_type();
Type *get_base_type(Type *type);
void do_typedef(char *tag, int len, Type *type);

/* variable.c */
Var *find_var(Token *request);
Var *regist_var(int var_type);

/* string.c */
int find_str(char *target_str);

/* codegen.c */
void gen_asm();

/* precalc.c */
int precalc_expr(Node *root);
void decode_precalc_expr(char *s, Node *root);

/* struct.c */
bool def_struct(int type, char *tag, int len);
int get_struct_size(char *tag, int len);
Var *struct_get_member(char *tag, int tag_len, char *member_n, int mn_len);

#endif // YNCC_H
