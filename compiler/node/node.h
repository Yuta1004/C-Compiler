#ifndef NODE_H
#define NODE_H

/* 列挙体 */
typedef enum {
    ND_NONE,
    ND_EQ,              // ==
    ND_NEQ,             // !=
    ND_UPPERL,          // >
    ND_UPPEREQL,        // >=
    ND_ADD,             // +
    ND_SUB,             // -
    ND_MUL,             // *
    ND_DIV,             // /
    ND_DIV_REMAIN,      // %
    ND_PRE_INC,         // 前置インクリメント
    ND_PRE_DEC,         // 前置デクリメント
    ND_POST_INC,        // 後置インクリメント
    ND_POST_DEC,        // 後置デクリメント
    ND_ASSIGN,          // =
    ND_ADDR,            // &()
    ND_DEREF,           // *()
    ND_LVAR,            // ローカル変数
    ND_GVAR,            // グローバル変数
    ND_INIT_GVAR,       // グローバル変数初期化
    ND_INIT_ARRAY,      // 配列初期化式
    ND_FUNC,            // 関数
    ND_CALL_FUNC,       // 関数呼び出し
    ND_BLOCK,           // ブロック
    ND_RETURN,          // return
    ND_IF,              // if
    ND_WHILE,           // while
    ND_DO_WHILE,        // do_while
    ND_FOR,             // for
    ND_BREAK,           // break
    ND_CONTINUE,        // continue
    ND_BIT_AND,         // &
    ND_BIT_OR,          // |
    ND_BIT_XOR,         // ^
    ND_BIT_SHIFT_L,     // <<
    ND_BIT_SHIFT_R,     // >>
    ND_BIT_NOT,         // ~
    ND_BOOL_AND,        // &&
    ND_BOOL_OR,         // ||
    ND_NUM,             // 数字
    ND_STR,             // 文字列リテラル
} NodeKind;

/* 構造体 */
typedef struct Node Node;
struct Node {
    NodeKind kind;              // ノードの種類
    Node *left;                 // 左辺ノードのポインタ
    Node *right;                // 右辺ノードのポインタ
    Vector *node_list;          // ノードに属するノードのリスト
    int val;                    // 数字ノードだった時、その値
    int offset;                 // ローカル変数ノードだった時、そのオフセット
    Type *type;                 // 変数ノードだった時、その型
    char *name;                 // グローバル変数定義 or 関数ノードだった時、その名前
    Node **args;                // 関数ノードだった時、その引数
};

/* 関数 */
Node *new_node(NodeKind kind);
Node *new_node_lr(NodeKind kind, Node *left, Node *right);
Node *new_num_node(int val);
Node *new_var_node(Var *var);
Node *new_none_node();

#endif
