#include<ctype.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<iostream>
#include "../include/tokenize.hpp"




int lvar_len(char *p){
    int len = 0;
    for(;;){
        if(('a' <= *p && *p <= 'z')||('A'<=*p && *p<='Z'||'0'<=*p && *p <= '9')){
            p++;
            len++;
        }else{
            return len;
        }
    }
}

int is_alnum(char c){
    return  ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_');
}

//pが指名しているボジ列をトークンに分割してtokensに保存する
void tokenize(char *p){
    int i = 0;
    Token end;
    char sete[] = "==",setne[] = "!=",setle[] = "<=",setle_re[] = ">=",hex[] = "0x",bin[] = "0b";
    while(*p){
        Token token;
        //空白文字をスキップ
        if(isspace(*p)||*p=='\n'||*p=='\t'){
            p++;
            continue;
        }

        // 0xの判別
        if(!memcmp(hex,p,2)){
            p+=2;
            token.ty = TK_NUM;
            token.str = p;
            token.val = strtol(p,&p,16);
            tokens.push_back(token);
            continue;
        }

        // 0bの判別
        if(!memcmp(bin,p,2)){
            p+=2;
            token.ty = TK_NUM;
            token.str = p;
            token.val = strtol(p,&p,2);
            tokens.push_back(token);
            continue;
        }

        // arr1 == arr2 の時の戻り値が０
        // == 
        if(!memcmp(sete,p,2)){    
            token.ty = TK_SETE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p+=2;
            continue;
        }
        // !=
        if(!memcmp(setne,p,2)){    
            token.ty = TK_SETNE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p+=2;
            continue;
        }
        // <= or >=
        if(!memcmp(setle,p,2)||!memcmp(setle_re,p,2)){    
            token.ty = TK_SETLE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p+=2;
            continue;
        }
        
        if(*p == '(' || *p == ')' || *p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='<' || *p=='>' || *p=='=' || *p == ';' || *p=='{' || *p=='}' || *p==',' || *p=='&' || *p=='[' || *p==']'){    
            token.ty = *p;
            token.str = p;
            token.len = 1;
            tokens.push_back(token);
            p++;
            continue;
        }



        //10進数の数字であるか否か
        if(isdigit(*p)){
            token.ty = TK_NUM;
            token.str = p;
            token.val = strtol(p,&p,10);
            tokens.push_back(token);
            continue;
        }

        // loop文の判別
        if(strncmp(p,"loop",4)==0&&!is_alnum(p[4])){
            token.ty = TK_LOOP;
            token.str = p;
            p+=4;
            tokens.push_back(token);
            continue;
        }

        // return文の判別
        if(strncmp(p,"return",6)==0&&!is_alnum(p[6])){
            token.ty = TK_RETURN;
            token.str = p;
            p+=6;
            tokens.push_back(token);
            continue;
        }

        //if文の判別
        if(strncmp(p,"if",2)==0&&!is_alnum(p[2])){
            token.ty = TK_IF;
            token.str = p;
            p+=2;
            tokens.push_back(token);
            continue;
        }
        //else文の判別
        if(strncmp(p,"else",4)==0&&!is_alnum(p[4])){
            token.ty = TK_ELSE;
            token.str = p;
            p+=4;
            tokens.push_back(token);
            continue;
        }
        //while文の判別
        if(strncmp(p,"while",5)==0&&!is_alnum(p[5])){
            token.ty = TK_WHILE;
            token.str = p;
            p+=5;
            tokens.push_back(token);
            continue;
        }

        // charの判別
        if(strncmp(p,"char",4)==0&&!is_alnum(p[4])){
            token.ty = TK_CHAR;
            token.str = p;
            p+=4;
            tokens.push_back(token);
            continue;
        }

        // shortの判別
        if(strncmp(p,"short",5)==0&&!is_alnum(p[5])){
            token.ty = TK_SHORT;
            token.str = p;
            p+=5;
            tokens.push_back(token);
            continue;
        }


        
        //変数の判別
        if('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z'){
            token.ty = TK_IDENT;
            token.str = p;
            token.len = lvar_len(p);
            p += token.len;  // ここでたしてあげないと文字数分ずれない。
            tokens.push_back(token);
            continue;
        }


        fprintf(stderr,"トークンナイズできません: %s\n",p);
        exit(1);
    }
    end.ty = TK_EOF;
    tokens.push_back(end);
    
    return;
}
