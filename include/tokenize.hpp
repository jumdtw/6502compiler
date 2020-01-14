#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include<vector>

enum {
    TK_NUM = 73947,  //整数トークン
    // 構文
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_LOOP,
    // 変数の型
    TK_SHORT, // short
    TK_CHAR,   // char
    TK_IDENT,     // 識別子　変数の名前とか
    // 比較演算子
    TK_SETE,  // ==
    TK_SETL,  // <
    TK_SETLE, // <=
    TK_SETNE, // !=
    // 進数　base number
    TK_HEX,
    TK_BIN,
    TK_EOF,        //入力の終わりを表すトークン
};

typedef struct {
    int ty;         //トークン型
    int val;        //tyがTK_NUMの場合、その数値
    char *str;    //トークン文字列
    int len;
}Token;

extern std::vector<Token> tokens;


void tokenize(char *p);

#endif