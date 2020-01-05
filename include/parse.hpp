#ifndef PARSE_HPP
#define PARSE_HPP


#include<vector>

enum{
    ND_NUM = 4334,
    ND_POINTER,
    ND_ADDER,
    // 比較演算
    ND_ASSIGN,
    ND_POINTER_ASSIGN, // 0xff以下
    ND_POINTER_ASSIGN_ABS, // 0xffより上（別に0xff以下でも使える）
    ND_RETURN,
    ND_IF,
    ND_ELSE,
    ND_WHILE,
    ND_FUNC,
    ND_FUNC_DEFINE,
    ND_LVAR,
    ND_BLOCK,
    ND_CHAR,
    ND_SHORT,
    ND_SETE,  // ==
    ND_SETL,  // <
    ND_SETLE, // <=
    ND_SETNE, // !=
};


typedef struct Node Node;
typedef struct LVar LVar;
typedef struct LFunc LFunc;
typedef struct Type Type;

struct Node{

    int ty;
    int val;
    int offset;
    int len;
    Node *lhs;
    Node *rhs;
    std::vector<Node*> stmts;
    char *str;
};

struct Type{
    int type;
    Type *ptr_to;
    int array_size;
};

struct LVar{
    char *name; //変数名
    int len;    //name.len()
    int val;
    int offset; //
    Type *lvar_type;  // 変数の型
};

struct LFunc{
    int len;
    // 構文木がここにはいっている
    std::vector<Node*> code;
    // ローカル変数リスト
    std::vector<LVar*> lvar_locals;
    char *name;
    Type *lvar_type;  // 関数の型

};

// ローカル関数リスト
extern std::vector<LVar*> locals;
// 関数リスト
extern std::vector<LFunc*> funcs;

#endif 