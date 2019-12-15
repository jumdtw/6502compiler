#ifndef PARSE_HPP
#define PARSE_HPP


#include<vector>

enum{
    ND_NUM = 4334,
    // 比較演算
    ND_ASSIGN,
    ND_RETURN,
    ND_IF,
    ND_ELSE,
    ND_WHILE,
    ND_FUNC,
    ND_FUNC_DEFINE,
    ND_LVAR,
    ND_BLOCK,
    ND_INT,
    ND_DOUBLE,
    ND_ADDR, //&
    ND_DEREF,//* アドレス参照
    ND_SETE,  // ==
    ND_SETL,  // <
    ND_SETLE, // <=
    ND_SETNE, // !=
};


typedef struct Node Node;
typedef struct LVar LVar;
typedef struct LFunc LFunc;

struct Node{

    int ty;
    int val;
    int offset;
    int len;
    std::vector<Node*> stmts;
    Node *lhs;
    Node *rhs;
    char *str;
};



struct LVar{
    char *name; //変数名
    int type;  // 変数の型
    int len;    //name.len()
    int val;
    int offset; //
};

struct LFunc{
    int type;  // 関数の型
    int len;
    // 構文木がここにはいっている
    std::vector<Node*> code;
    // ローカル変数リスト
    std::vector<LVar*> lvar_locals;
    char *name;

};

#endif 